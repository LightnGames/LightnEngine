#include <iostream>
#include <fstream>
#include <fbxsdk.h>
#include <algorithm>
#include <chrono>
#include <string>
#include <unordered_map>
#include <unordered_map>
#include <Renderer/Mesh/Mesh.h>
#include <Loader/MeshLoader.h>
#include <Renderer/Mesh/SkeletalMesh.h>
#include <Animation/SkeletalAnimation.h>
#include <Util/Type.h>
#include <cassert>

using namespace fbxsdk;

fbxsdk::FbxManager* manager;
fbxsdk::FbxScene* scene;
fbxsdk::FbxImporter* importer;

bool animationLoop = true;
int indexOffset = 0;
int materialOffset = 0;
std::vector<MeshVertex> publicVertex;
std::vector<SKVertex> publicSKVertex;
std::unordered_map<std::string, std::vector<int>> _nIndex;
MeshType _type;

std::unique_ptr<Skeleton> _skeleton;
int dii[MAX_BONES];

bool isSkeletalMesh() {
	return _type == MeshType::Skeletal;
}

Vector3 CastFromFbxVector(const FbxVector4& fbxV){

	return Vector3(fbxV[0], fbxV[1], fbxV[2]);
}

Matrix4 castFromFbxMatrix(const fbxsdk::FbxMatrix & matrix) {

	Matrix4 result;
	//FBXMatrixから変換
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			result.m[y][x] = matrix.Get(y, x);
		}
	}
	return result;
}

Quaternion CastFromFbxQuaternion(const FbxQuaternion& fbxQ){
	return Quaternion(fbxQ[0], fbxQ[1], fbxQ[2], fbxQ[3]);
}

bool checkAttribute(const fbxsdk::FbxNode * node, const FbxNodeAttribute::EType type){

	//属性数を取得
	const int attrCount = node->GetNodeAttributeCount();

	for(int i = 0; attrCount > i; i++){
		FbxNodeAttribute::EType attrType = node->GetNodeAttributeByIndex(i)->GetAttributeType();

		// ノード属性が一致
		if(attrType == type){
			return true;
		}
	}

	//なにも当てはまらなかったので一致していない
	return false;
}

std::vector<FbxNode*> childNode(FbxNode* parent, const FbxNodeAttribute::EType type, bool rootNode){

	int nodeCount = parent->GetChildCount();
	std::vector<FbxNode*> childlen;

	for(int i = 0; i < nodeCount; ++i){
		FbxNode* child = parent->GetChild(i);

		if(checkAttribute(child, type)){
			childlen.emplace_back(child);
		}
		else{
			if(!rootNode){
				continue;
			}
		}

		auto nextChildlen = childNode(child, type, false);
		childlen.insert(childlen.end(), nextChildlen.begin(), nextChildlen.end());
	}

	return childlen;
}

void setVertexTexCoord(const FbxVector2 & v, std::vector<MeshVertex>& vertices, int index) {

	vertices[index].tex.x = static_cast<float>(v[0]);
	vertices[index].tex.y = 1.0f - static_cast<float>(v[1]);
}

std::unique_ptr<Skeleton> loadSkeleton() {

	//FBX初期化チェック
	if (manager == nullptr) {
		return nullptr;
	}

	if (_skeleton.get() != nullptr) {
		return std::move(_skeleton);
	}

	//スケルトン生成
	auto skeleton = std::make_unique<Skeleton>();

	std::vector<Bone> bones(MAX_BONES);

	//メッシュ数を取得
	const int meshCount = scene->GetMemberCount<FbxMesh>();

	//複数メッシュ共有のスケルトンノードを羅列(比較用)
	const auto childlen = childNode(scene->GetRootNode(), FbxNodeAttribute::EType::eSkeleton, true);

	//メッシュの数だけロード
	for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex) {

		FbxMesh* mesh = scene->GetMember<FbxMesh>(meshIndex);

		//スケールを単位化する行列を作成
		const FbxDouble3 fScale = mesh->GetNode()->LclScaling.Get();
		const Matrix4 mtxScale = Matrix4::scaleXYZ(1 / fScale[0], 1 / fScale[1], 1 / fScale[2]);

		//ボーンを取得
		FbxSkin* pSkinInfo = static_cast<FbxSkin*>(mesh->GetDeformer(0));
		const UINT numBone = pSkinInfo->GetClusterCount();

		//カレントボーン行列を取得
		for (UINT i = 0; i < numBone; ++i) {

			FbxCluster* cluster = pSkinInfo->GetCluster(i);
			const std::string clusterName = cluster->GetLink()->GetName();

			//既に同名のボーンがないか調査する
			int boneIndex = 0;
			for (auto& b : childlen) {
				if (b->GetName() == clusterName) {
					break;
				}
				boneIndex++;
			}

			if (bones[boneIndex].name == "") {
				FbxAMatrix mat;
				cluster->GetTransformLinkMatrix(mat);

				Bone bone;
				bone.name = clusterName;
				bone.matrix = Matrix4::multiply(castFromFbxMatrix(mat), Matrix4::multiply(Matrix4::rotateY(180), mtxScale));
				bones[boneIndex] = bone;
			}
		}
	}

	for (int i = 0; i < childlen.size(); ++i) {
		skeleton->boneMatrices.emplace_back(bones[i]);
	}

	return skeleton;
}

HRESULT loadLocalMesh(const int index) {

	double loadTime = 0;

	FbxMesh* mesh = scene->GetMember<FbxMesh>(index);

	const int controlCount = mesh->GetControlPointsCount();
	const int uvCount = mesh->GetTextureUVCount();
	const int faceCount = mesh->GetPolygonCount();
	const int maxIndexCount = controlCount < uvCount ? uvCount : controlCount;

	std::vector<MeshVertex> vertices(maxIndexCount);
	std::vector<std::vector<int>> rawIndices(controlCount);

	//==================================================================================
	std::chrono::system_clock::time_point  start, end; // 型は auto で可
	start = std::chrono::system_clock::now(); // 計測開始時間

											  //ポリゴン取得
	for (int i = 0; i < faceCount; ++i) {

		const int startIndex = mesh->GetPolygonVertexIndex(i);
		const int* pIndex = mesh->GetPolygonVertices();

		const FbxVector4* coord = mesh->GetControlPoints();
		const FbxLayerElementUV* uv = mesh->GetLayer(0)->GetUVs();

		//ポリゴン内の頂点数でループ
		for (int k = 0; k < 3; ++k) {
			//ポリゴン内でのインデックス
			int faceIndex = pIndex[startIndex + k];
			int faceIndexRaw = faceIndex;

			//UVの数の方がでかい場合
			if (controlCount < uvCount) {
				faceIndex = mesh->GetTextureUVIndex(i, k, FbxLayerElement::eTextureDiffuse);
			}

			//まだ登録されていないインデックスなら登録
			const auto found = std::find(rawIndices[faceIndexRaw].begin(), rawIndices[faceIndexRaw].end(), faceIndex);
			if (found == rawIndices[faceIndexRaw].end()) {
				rawIndices[faceIndexRaw].push_back(faceIndex);
			}

			//左手座標系変換のためにZを反転する
			//頂点座標
			const int vertexIndex = mesh->GetPolygonVertex(i, k);
			vertices[faceIndex].pos.x = static_cast<float>(coord[vertexIndex][0]);
			vertices[faceIndex].pos.y = static_cast<float>(coord[vertexIndex][1]);
			vertices[faceIndex].pos.z = static_cast<float>(coord[vertexIndex][2]);

			//法線
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(i, k, normal);
			vertices[faceIndex].normal.x = static_cast<float>(normal[0]);
			vertices[faceIndex].normal.y = static_cast<float>(normal[1]);
			vertices[faceIndex].normal.z = static_cast<float>(normal[2]);
			vertices[faceIndex].normal = Vector3::normalize(vertices[faceIndex].normal);

			//Tangent
			//Binormal
			const Vector3 calcNormal = vertices[faceIndex].normal;
			const Vector3 vectorUp = { 0.0f, 1, 0.000001f };
			vertices[faceIndex].tangent = Vector3::cross(calcNormal, vectorUp);
			vertices[faceIndex].binormal = Vector3::cross(vertices[faceIndex].tangent, calcNormal);

			//UVインデックスを取得
			int uvIndex = mesh->GetTextureUVIndex(i, 0, FbxLayerElement::eTextureDiffuse);

			//UV専用の頂点座標インデックスがある場合は頂点バッファに追加する
			//該当しない場合はそのまま取れたインデックスを使う(UVインデックスと頂点インデックスが同じということ)
			if (uvCount&&uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
				uvIndex = mesh->GetTextureUVIndex(i, k, FbxLayerElement::eTextureDiffuse);

				const FbxLayerElementUV* pUV = mesh->GetLayer(0)->GetUVs();
				const FbxVector2 v2 = pUV->GetDirectArray().GetAt(uvIndex);
				setVertexTexCoord(v2, vertices, faceIndex);
			}
		}
	}

	//UV取得
	FbxLayerElementUV* uv = mesh->GetLayer(0)->GetUVs();
	if (uvCount&&uv->GetMappingMode() == FbxLayerElement::eByControlPoint) {
		FbxLayerElementUV* pUV = mesh->GetLayer(0)->GetUVs();
		for (int i = 0; i < uvCount; ++i) {
			const FbxVector2 v2 = pUV->GetDirectArray().GetAt(i);
			setVertexTexCoord(v2, vertices, i);
		}
	}

	//読み込んだメッシュを回転
	const FbxMatrix meshTransform = mesh->GetNode()->EvaluateGlobalTransform();
	const FbxDouble3 fScale = mesh->GetNode()->LclScaling.Get();
	const FbxDouble3 rotetor = mesh->GetNode()->LclRotation.Get();
	const FbxDouble3 translate = mesh->GetNode()->LclTranslation.Get();

	//fbxsdkは右手座標系なので左手座標系に直す&メッシュのスケールを単位化
	const Matrix4 scaleZ = Matrix4::scaleXYZ(1 / fScale[0], 1 / fScale[1], -1 / fScale[2]);
	const Matrix4 scale = Matrix4::scaleXYZ(1 / fScale[0], 1 / fScale[1], 1 / fScale[2]);
	Matrix4 inv = castFromFbxMatrix(meshTransform);
	inv = Matrix4::multiply(inv, Matrix4::rotateX(180));

	//変換を実行
	for (auto& v : vertices) {
		v.MeshTransform(Matrix4::multiply(inv, scaleZ));
	}

	//現在のノードからマテリアル数を取得
	const UINT materialCount = mesh->GetNode()->GetMaterialCount();
	//マテリアルの数だけチェック
	int indexCount = 0;
	for (UINT i = 0; i < materialCount; ++i) {

		int iCount = 0;
		std::vector<int> pIndex(faceCount * 3);

		//再度ポリゴンの数だけループ
		for (int j = 0; j < faceCount; ++j) {
			FbxLayerElementMaterial* mat = mesh->GetLayer(0)->GetMaterials();
			const int matId = mat->GetIndexArray().GetAt(j);

			//そのポリゴンのマテリアルが現在ループ中のマテリアルと一致していたらインデックスバッファに追加
			//左手座標系対応のため123を132の順番で登録
			if (matId == i) {
				if (controlCount < uvCount) {
					pIndex[iCount] = mesh->GetTextureUVIndex(j, 0, FbxLayerElement::eTextureDiffuse) + indexOffset;
					pIndex[iCount + 2] = mesh->GetTextureUVIndex(j, 1, FbxLayerElement::eTextureDiffuse) + indexOffset;
					pIndex[iCount + 1] = mesh->GetTextureUVIndex(j, 2, FbxLayerElement::eTextureDiffuse) + indexOffset;
				} else {
					pIndex[iCount] = mesh->GetPolygonVertex(j, 0) + indexOffset;
					pIndex[iCount + 2] = mesh->GetPolygonVertex(j, 1) + indexOffset;
					pIndex[iCount + 1] = mesh->GetPolygonVertex(j, 2) + indexOffset;
				}

				iCount += 3;
			}
		}

		const std::string matName = mesh->GetNode()->GetMaterial(i)->GetName();

		//すでにマテリアルをロードしていたらインデックスバッファのみ追加する
		if (_nIndex.count(matName) > 0) {
			auto& mIndex = _nIndex[matName];
			mIndex.reserve(mIndex.size() + pIndex.size());
			std::copy(pIndex.begin(), pIndex.end(), std::back_inserter(mIndex));

		} else {

			_nIndex[matName] = std::move(pIndex);
		}
		//マテリアルごとのインデックスバッファ作成
		//createIndexBuffer(pIndex.data(), iCount, material->pIndexBuffer);
		indexCount += iCount;
	}

	materialOffset += materialCount;
	indexOffset += vertices.size();

	//スケルタルメッシュでなければ頂点バッファを生成して終了
	if (!isSkeletalMesh()) {
		publicVertex.reserve(publicVertex.size() + vertices.size());
		std::copy(vertices.begin(), vertices.end(), std::back_inserter(publicVertex));
		//createVertexBuffer(vertices.data(), sizeof(MeshVertex)*maxIndexCount, _meshes[index].vertexBuffer);
		return S_OK;
	}

	//FBXロード900ms
	loadTime += 0;

	//ボーンを取得
	FbxSkin* pSkinInfo = static_cast<FbxSkin*>(mesh->GetDeformer(0));
	const UINT numBone = pSkinInfo->GetClusterCount();
	_skeleton = loadSkeleton();

	std::vector<SKVertex> skVertices;
	skVertices.reserve(maxIndexCount);

	//スケルタルメッシュ用の頂点バッファにスタティックメッシュで取得したデータをコピー
	for (auto& v : vertices) {
		SKVertex vertex;
		vertex.pos = v.pos;
		vertex.tex = v.tex;
		vertex.normal = v.normal;
		vertex.tangent = v.tangent;
		vertex.binormal = v.binormal;

		skVertices.emplace_back(vertex);
	}

	//カレントボーン行列を取得
	for (UINT i = 0; i < numBone; ++i) {

		FbxCluster* cluster = pSkinInfo->GetCluster(i);
		const std::string clusterName = cluster->GetLink()->GetName();

		//既に同名のボーンがないか調査する
		int boneIndex = 0;
		for (auto&& b : _skeleton->boneMatrices) {
			if (b.name == clusterName) {
				break;
			}
			boneIndex++;
		}

		const int numIndex = cluster->GetControlPointIndicesCount();
		const int* pIndex = cluster->GetControlPointIndices();
		const double* pWeight = cluster->GetControlPointWeights();

		//ボーンにくっついているインデックスで走査
		for (int j = 0; j < numIndex; ++j) {

			//生のインデックス番号から現在の頂点バッファに紐づくインデックスで走査
			for (auto v : rawIndices[pIndex[j]]) {

				//ボーンのウェイトを4つまでセットする
				for (int k = 0; k < 4; ++k) {

					//ボーンのウェイトが割り当てられていないなら割り当て
					if (0.0f == skVertices[v].boneWeight[k]) {
						skVertices[v].boneIndex[k] = boneIndex;
						skVertices[v].boneWeight[k] = pWeight[j];

						break;
					}

					//ウェイトが4つ以上検出された場合
					if (k == 3) {
						int minIndex = 0;
						float minWeight = 1.0f;
						for (int l = 0; l < 4; ++l) {
							if (skVertices[v].boneWeight[k] < minWeight) {
								minIndex = l;
								minWeight = skVertices[v].boneWeight[k];
							}
						}

						if (pWeight[j] > minWeight) {
							skVertices[v].boneIndex[minIndex] = boneIndex;
							skVertices[v].boneWeight[minIndex] += pWeight[j];
						}
					}
				}
			}
		}
	}

	int count = 0;
	for (auto&& v : skVertices) {
		if (v.boneWeight[0] + v.boneWeight[1] + v.boneWeight[2] + v.boneWeight[3] < 0.7f) {

			if ((v.pos.x != 0.0f) && (v.tex.x > 0)) {
				v.boneWeight[0] = skVertices[count-1].boneWeight[0];
				v.boneWeight[1] = skVertices[count-1].boneWeight[1];
				v.boneWeight[2] = skVertices[count-1].boneWeight[2];
				v.boneWeight[3] = skVertices[count-1].boneWeight[3];

				v.boneIndex[0] = skVertices[count-1].boneIndex[0];
				v.boneIndex[1] = skVertices[count-1].boneIndex[1];
				v.boneIndex[2] = skVertices[count-1].boneIndex[2];
				v.boneIndex[3] = skVertices[count-1].boneIndex[3];
			}
		}

		count++;
	}

	dii;
	_skeleton->boneMatrices;
	publicSKVertex.reserve(publicSKVertex.size() + skVertices.size());
	std::copy(skVertices.begin(), skVertices.end(), std::back_inserter(publicSKVertex));

	return S_OK;
}

void writeMaterialData(std::ofstream& fout, const char* meshType) {

	//メッシュタイプ
	fout.write(reinterpret_cast<const char*>(meshType), 2);

	//マテリアル数
	const int mCount = _nIndex.size();
	fout.write(reinterpret_cast<const char*>(&mCount), 4);

	for (auto&& m : _nIndex) {

		//マテリアル名前とポリゴン数
		const int fCount = m.second.size() / 3;

		char materialName[32] = {};
		memcpy(materialName, m.first.c_str(), m.first.length());
		fout.write(reinterpret_cast<const char*>(materialName), 32);
		fout.write(reinterpret_cast<const char*>(&fCount), 4);

		//インデックスの数とデータ本体
		const int indexSize = m.second.size();
		fout.write(reinterpret_cast<const char*>(&indexSize), 4);
		fout.write(reinterpret_cast<const char*>(m.second.data()), indexSize * sizeof(int));
	}
}

template<class T>
void createAABB(const T& array,Vector3& first, Vector3& end) {

	for (auto&& v : array) {
		const Vector3& p = v.pos;
		if (p.x < first.x) { first.x = p.x; }
		if (p.x > end.x) { end.x = p.x; };
		if (p.y < first.y) { first.y = p.y; }
		if (p.y > end.y) { end.y = p.y; };
		if (p.z < first.z) { first.z = p.z; }
		if (p.z > end.z) { end.z = p.z; };
	}
}

void loadMesh(const std::string& meshFilePath) {

	FbxAxisSystem::DirectX.ConvertScene(scene);

	//三角形ポリゴンに変換
	FbxGeometryConverter geometoryConverter(manager);
	geometoryConverter.Triangulate(scene, true);

	//メッシュ数を取得
	const int meshCount = scene->GetMemberCount<FbxMesh>();

	//メッシュの数だけロード
	for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		loadLocalMesh(meshIndex);
	}

	dii;
	_skeleton->boneMatrices;
	publicSKVertex;

	int index = 0;
	float minW = 1.0f;
	for (auto&& v : publicSKVertex) {
		float w = v.boneWeight[0] + v.boneWeight[1] + v.boneWeight[2] + v.boneWeight[3];
		if (w < minW) {
			minW = w;

			if (w == 0.0f) {
				int out = 1;
			}
		}
		index++;
	}

	std::ofstream fout;
	fout.open(meshFilePath + "mesh", std::ios::out | std::ios::binary | std::ios::trunc);

	//バウンディングボックス
	Vector3 first = Vector3::zero;
	Vector3 end = Vector3::zero;
	uint32 binSize;
	char* vertexPtr = nullptr;

	if (_type == MeshType::Static) {

		writeMaterialData(fout, "SM");

		//頂点バッファサイズとデータ本体
		binSize = sizeof(MeshVertex)*publicVertex.size();
		vertexPtr = reinterpret_cast<char*>(publicVertex.data());

		//メッシュのAABB境界を計算
		createAABB<std::vector<MeshVertex>>(publicVertex, first, end);

	} else if (_type == MeshType::Skeletal) {

		writeMaterialData(fout, "SK");

		//頂点バッファサイズとデータ本体
		binSize = sizeof(SKVertex)*publicSKVertex.size();
		vertexPtr = reinterpret_cast<char*>(publicSKVertex.data());

		//メッシュのAABB境界を計算
		createAABB<std::vector<SKVertex>>(publicSKVertex, first, end);
	}

	char* baka = "BAKA";
	uint32 align = binSize % 4;
	fout.write(reinterpret_cast<char*>(&binSize), 4);
	fout.write(reinterpret_cast<char*>(vertexPtr), binSize);

	fout.write(reinterpret_cast<char*>(&first), sizeof(Vector3));
	fout.write(reinterpret_cast<char*>(&end), sizeof(Vector3));
	//fout.write(reinterpret_cast<char*>(baka), 4);

	std::cout << "first X:" << first.x << " Y:" << first.y << " Z:" << first.z << std::endl;
	std::cout << "end X:" << end.x << " Y:" << end.y << " Z:" << end.z << std::endl;


	if (_type == MeshType::Skeletal) {

		//スケルトン情報を書き込み
		uint32 skeletonSize = _skeleton->boneMatrices.size();
		fout.write(reinterpret_cast<char*>(&skeletonSize), 4);
		for (auto&& s : _skeleton->boneMatrices) {

			//ボーン名
			char boneName[32] = {};
			memcpy(boneName, s.name.c_str(), s.name.length());
			fout.write(reinterpret_cast<char*>(boneName), 32);

			//バインドポーズ行列
			fout.write(reinterpret_cast<char*>(&s.matrix), 64);
		}
	}

	fout.close();
}

bool checkEnableAnimCurve(const FbxAnimCurve* curve) {
	if (curve == nullptr) { return false; }
	if (curve->KeyGetCount() < 2) { return false; }
	return true;
}

void loadAnim(const std::string& meshFilePath){

	std::vector<AnimationBone> _animationBones;

	double loadTime = 0;
	std::chrono::system_clock::time_point  start, end; // 型は auto で可
	start = std::chrono::system_clock::now(); // 計測開始時間

	//いったん右手座標系で統一(FBXの仕様に合わせる)
	FbxAxisSystem::OpenGL.ConvertScene(scene);

	//Rootノードとその子ノードを列挙
	FbxNode* rootNode = scene->GetRootNode();
	auto childlen = childNode(rootNode, FbxNodeAttribute::EType::eSkeleton, true);

	//アニメーションカーブ取得
	FbxAnimStack* pAnimStack = scene->GetSrcObject<FbxAnimStack>(0);
	FbxAnimLayer* lAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(0);
	FbxAnimCurve* curve = nullptr;

	for (int i = 0; i < childlen.size(); ++i) {
		curve = childlen[i]->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (checkEnableAnimCurve(curve)) { break; }
		curve = childlen[i]->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (checkEnableAnimCurve(curve)) { break; }
		curve = childlen[i]->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (checkEnableAnimCurve(curve)) { break; }
	}

	assert(curve != nullptr&&"アニメーションカーブが見つかりません");

	//キーフレームの間隔からアニメーションフレームレートを割り出す
	const int keyCount = curve->KeyGetCount();
	const int maxKeyIndex = keyCount - 1;
	double keyFramerate = curve->KeyGetTime(maxKeyIndex).GetSecondDouble();
	keyFramerate /= maxKeyIndex;
	keyFramerate = 1 / keyFramerate;

	//アニメーションフレームレートを30fpsに強制した最大数に変換
	const int keyCountPer30fps = keyCount * 30.0f / keyFramerate - 1;

	//FbxTime::EMode mode = FbxTime::ConvertFrameRateToTimeMode(keyFramerate);

	_animationBones.reserve(childlen.size());

	int count = 0;

	std::cout << "総フレーム数 " << keyCountPer30fps << std::endl;

	//骨の数だけループ
	for(auto&& b : childlen){

		AnimationBone animBone;
		animBone.keys.reserve(keyCountPer30fps);

		count++;
		std::cout << "変換中　" << childlen.size() << " / " << count << std::endl;

		for(int i = 0; i < keyCountPer30fps; ++i){

			//キーフレームを30fpsに変換
			FbxTime period;
			period.SetTime(0, 0, 0, i, 0, FbxTime::EMode::eFrames30);

			//キーフレームの行列を取得
			const FbxAMatrix matrix = b->EvaluateGlobalTransform(period);

			//キーフレーム生成・各成分を抽出
			TransformQ keyFrame;
			keyFrame.position = CastFromFbxVector(matrix.GetT());
			keyFrame.rotation = CastFromFbxQuaternion(matrix.GetQ());
			keyFrame.scale = CastFromFbxVector(matrix.GetS());

			//右手座標を左手座標に変換
			keyFrame.rotation.z *= -1;
			keyFrame.rotation.w *= -1;
			keyFrame.scale.z *= -1;
			keyFrame.position.z *= -1;

			animBone.keys.emplace_back(keyFrame);
		}

		_animationBones.emplace_back(animBone);
	}

	end = std::chrono::system_clock::now();  // 計測終了時間
	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
	loadTime += elapsed;

	std::ofstream fout;
	fout.open(meshFilePath + "anim", std::ios::out | std::ios::binary | std::ios::trunc);
	//  ファイルを開く
	//  ios::out は書き込み専用（省略可）
	//  ios::binary はバイナリ形式で出力（省略するとアスキー形式で出力）
	//  ios::truncはファイルを新規作成（省略可）
	//  ios::addにすると追記になる

	//最初にボーン数を書き込み
	int boneSize = _animationBones.size();
	fout.write(reinterpret_cast<char*>( &boneSize ), sizeof(int));

	//ボーンのキーフレームの数を書き込み
	int keySize = _animationBones[0].keys.size();
	fout.write(reinterpret_cast<char*>( &keySize ), sizeof(int));

	//30fps時のフレーム数
	int keySizePer30fps = keyCountPer30fps;
	fout.write(reinterpret_cast<char*>( &keySizePer30fps ), sizeof(int));

	//各ボーンに存在するキーフレームを書き込み
	for(auto&& b : _animationBones){

		for(auto&& k : b.keys){
			fout.write(reinterpret_cast<char*>( &k.position ), sizeof(Vector3));
			fout.write(reinterpret_cast<char*>( &k.rotation ), sizeof(Quaternion));
			fout.write(reinterpret_cast<char*>( &k.scale ), sizeof(Vector3));
		}
	}

	fout.close();  //ファイルを閉じる
}

int main(int argc, char *argv[]){
	
	//argc = 2;
	//argv[1] = "C:/Users/Dunois/Documents/VisualStudio Projects/LightnEngine/LightnLibrary/LightnLibrary/Resources/Hero/Hero_idle.fbx";
	if(argc > 1){
		for(int i = 1; i<argc; i++){

			std::string fileName(argv[i]);

			std::cout << fileName<< " 処理開始" << std::endl;

			indexOffset = 0;
			materialOffset = 0;
			publicVertex = std::vector<MeshVertex>();
			publicSKVertex = std::vector<SKVertex>();
			_nIndex = std::unordered_map<std::string, std::vector<int>>();
			_type = MeshType::None;

			std::unique_ptr<Skeleton> _skeleton;

			//メッシュパスを取得
			std::string fullpath(fileName.c_str(), strlen(fileName.c_str()));
			int path_i = fullpath.find_last_of(".") + 1;
			std::string meshFilePath = fullpath.substr(0, path_i);

			//マネージャーとインポーター生成
			manager = FbxManager::Create();
			importer = FbxImporter::Create(manager, "importer");

			//ファイル名からFBX読み込み
			if (!importer->Initialize(fileName.c_str())) {
				throw std::runtime_error("failed load mesh file ");
				return 0;
			}

			//シーンを生成して読み込んだモデルをセット
			scene = FbxScene::Create(manager, "");
			importer->Import(scene);

			//メッシュタイプをチェック
			const bool includeSkeleton = scene->GetMemberCount<FbxSkeleton>() != 0;
			const bool includeMesh = scene->GetMemberCount<FbxMesh>() != 0;

			std::string debugMeshType;

			if (includeMesh && !includeSkeleton) {
				_type = MeshType::Static;
				debugMeshType = "スタティックメッシュ";
			}

			if (includeMesh && includeSkeleton) {
				_type = MeshType::Skeletal;
				debugMeshType = "スケルタルメッシュ";
			}

			if (!includeMesh && includeSkeleton) {
				_type = MeshType::Animation;
				debugMeshType = "アニメーション";
			}

			std::cout << "ファイルタイプ： " << debugMeshType << std::endl;

			switch (_type) {

			case MeshType::Static:
			case MeshType::Skeletal:

				loadMesh(meshFilePath);
				break;

			case MeshType::Animation:
				loadAnim(meshFilePath);
				break;
			}

			importer->Destroy();
			scene->Destroy();
			manager->Destroy();
		}
	}

	std::cout << "変換完了" << std::endl;
	system("pause");
}