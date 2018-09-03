#include "MeshLoader.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/Mesh/SkeletalMesh.h>
#include <Renderer/GraphicsResourceManager.h>

#include <DirectXTex.h>
#include <iostream>
#include <fstream>
#include <cassert>

MeshLoader::MeshLoader(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& deviceContext)
	:_device{ device }, _deviceContext{ deviceContext } {
}

MeshLoader::~MeshLoader() {
}

HRESULT MeshLoader::load(const std::string & filePath, const std::vector<std::string>& matFiles) {

	//メッシュパスを取得
	const int path_slash = filePath.find_last_of("/") + 1;
	_folderPath = "Resources/" + filePath.substr(0, path_slash);
	_filePath = filePath;

	const int path_point = filePath.find_last_of(".") + 1;
	auto fileName = filePath.substr(0, path_point) + "mesh";

	//メッシュバイナリロード
	std::ifstream fin("Resources/" + fileName, std::ios::in | std::ios::binary);
	fin.exceptions(std::ios::badbit);
	
	if (fin.fail()) {
		int b = 0;
	}
	
	assert(!fin.fail());

	//マテリアルと頂点バッファロード
	loadMaterialAndVertexData(fin);

	if (getType() == MeshType::Skeletal) {

		//スケルトンロード
		int gcnt = 0;
		int boneCount = 0;
		fin.read(reinterpret_cast<char*>(&boneCount), 4);
		gcnt = fin.gcount();

		_skeleton = std::make_unique<Skeleton>(boneCount);

		for (int i = 0; i < boneCount; i++) {
			char boneName[32] = {};
			fin.read(reinterpret_cast<char*>(boneName), 32);

			Matrix4 matrix;
			fin.read(reinterpret_cast<char*>(&matrix), 64);

			_skeleton->boneMatrices[i].name = boneName;
			_skeleton->boneMatrices[i].matrix = matrix;
		}
	}

	fin.close();

	//マテリアルデータをファイルから読み込み
	loadMaterialFromFile(matFiles);
	setupMaterial();

	return S_OK;
}

void MeshLoader::loadMaterialAndVertexData(std::ifstream& fin) {

	//メッシュタイプ
	char meshType[2];
	fin.read(reinterpret_cast<char*>(meshType), 2);

	//マテリアル数
	int materialCount = 0;
	fin.read(reinterpret_cast<char*>(&materialCount), 4);

	//マテリアルの数だけ読み込み
	for (int i = 0; i < materialCount; i++) {

		char materialName[32] = {};
		fin.read(reinterpret_cast<char*>(materialName), 32);

		int faceCount = 0;
		fin.read(reinterpret_cast<char*>(&faceCount), 4);

		int indexCount = 0;
		fin.read(reinterpret_cast<char*>(&indexCount), 4);

		int indexSize = indexCount * sizeof(int);
		std::unique_ptr<char> indexBuffer = std::unique_ptr<char>(new char[indexSize]);
		fin.read(reinterpret_cast<char*>(indexBuffer.get()), indexSize);

		auto material = std::make_shared<MaterialData>();
		RendererUtil::createIndexBuffer(indexBuffer.get(), indexCount, material->pIndexBuffer, _device);
		material->faceCount = faceCount;

		_mesh.materialSlots.emplace_back(std::move(material));
	}

	try {
		//頂点バッファ読み込み
		int gcnt = 0;
		int vertexSize = 0;
		fin.read(reinterpret_cast<char*>(&vertexSize), 4);
		gcnt = fin.gcount();

		auto vertexBuf = std::unique_ptr<char>(new char[vertexSize]);
		fin.read(reinterpret_cast<char*>(vertexBuf.get()), vertexSize);
		gcnt = fin.gcount();

		char ppp[4];

		//バウンディングボックス読み込み
		Vector3 min;
		Vector3 max;
		//fin.read(reinterpret_cast<char*>(ppp), 4);
		gcnt = fin.gcount();
		fin.read(reinterpret_cast<char*>(&min), sizeof(Vector3));
		gcnt = fin.gcount();
		fin.read(reinterpret_cast<char*>(&max), sizeof(Vector3));
		gcnt = fin.gcount();

		_mesh.boundingBox.min = min;
		_mesh.boundingBox.max = max;

		//スタティックメッシュ頂点バッファ生成
		RendererUtil::createVertexBuffer(vertexBuf.get(), vertexSize, _mesh.vertexBuffer, _device);
	}
catch (std::ios_base::failure& e) {
	std::cerr << "file read error." << std::endl;
	}

	//メッシュ属性をメンバに格納
	_materialSlotCount = materialCount;
	if (meshType[0] == 'S'&&meshType[1] == 'M') {
		_type = MeshType::Static;
	} else if (meshType[0] == 'S'&&meshType[1] == 'K') {
		_type = MeshType::Skeletal;
	}
}

std::unique_ptr<Skeleton> MeshLoader::loadSkeleton() {
	return std::move(_skeleton);
}

HRESULT MeshLoader::setupMaterial() {

	for (int i = 0; i < _materialSlotCount; i++) {

		//個々のマテリアルポインタを生成
		auto& material = _mesh.materialSlots[i];

		//シェーダーと定数バッファを生成
		createShaderAndConstantBuffer(material, isSkeletalMesh());

		//テクスチャのシェーダーリソースビューを作成
		for (UINT j = 0; j < material->textureCount; ++j) {

			//テクスチャを取得
			const std::string fullPath = _folderPath + material->textureFileNames[j];
			auto texture = GraphicsResourceManager::instance().loadTexture(fullPath);

			//ミップマップを生成
			_deviceContext->GenerateMips(texture.Get());

			//マテリアルに登録
			material->ppTextures.emplace_back(texture);
		}

		//テクスチャ―サンプラーを作成
		material->pSamplerLiner = GraphicsResourceManager::instance().simpleSamplerState();
	}

	return S_OK;
}

HRESULT MeshLoader::loadMaterialFromFile(const std::vector<std::string>& fileName) {

	//マテリアルの数だけ走査
	for (int matIndex = 0; matIndex < fileName.size(); matIndex++) {

		std::string fullPath = _folderPath.c_str();
		fullPath += fileName[matIndex];
		std::ifstream ifs(fullPath);
		std::string str;

		auto& material = _mesh.materialSlots[matIndex];

		int textureCount = 0;
		int loadTextureCount = 0;
		int lineCount = 0;

		ifs.clear();
		ifs.seekg(0, std::ios::beg);

		//ファイルの行ごとに処理
		while (getline(ifs, str)) {

			switch (lineCount) {
			case 0:
				//頂点シェーダーファイル名取得
				material->vertexShaderFileName = std::string(str.c_str());
				break;
			case 1:
				//ピクセルシェーダー名取得
				material->pixelShaderFileName = std::string(str.c_str());
				break;
			case 2:
				//マテリアルアルファタイプを取得
				material->alphaType = std::stoi(str);
				break;
			case 3:
				//ポリゴン表示モード
				switch (std::stoi(str)) {
				case 0: material->cullMode = D3D11_CULL_BACK; break;
				case 1: material->cullMode = D3D11_CULL_NONE; break;
				case 2: material->cullMode = D3D11_CULL_FRONT; break;
				}
				break;
			case 4:
				//このマテリアルのテクスチャ枚数取得
				textureCount = std::stoi(str);
				material->textureCount = textureCount;
				break;
			default:
				//テクスチャの数だけファイル名を読み込み
				if (loadTextureCount < textureCount) {
					material->textureFileNames.push_back(std::string(str.c_str()));
					loadTextureCount++;
				}
				break;
			}

			lineCount++;
		}
	}

	return S_OK;
}

HRESULT MeshLoader::createShaderAndConstantBuffer(std::shared_ptr<MaterialData>& material, bool isSkeletalMesh) {

	//頂点インプットレイアウトを作成
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 });


	if (isSkeletalMesh) {
		layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BONE_INDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BONE_WEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,72,D3D11_INPUT_PER_VERTEX_DATA,0 });
	}


	//頂点シェーダーを読み込み
	RendererUtil::createVertexShader(material->vertexShaderFileName.c_str(), material->pVertexShader, layout.data(), layout.size(), material->pVertexLayout, _device);

	//コンスタントバッファー作成
	const UINT constantBufferSize = isSkeletalMesh ? sizeof(SkeletalMeshConstantBuffer) : sizeof(MeshConstantBuffer);
	RendererUtil::createConstantBuffer(material->pConstantBuffer, constantBufferSize, _device);

	//ピクセルシェーダー読み込み
	RendererUtil::createPixelShader(material->pixelShaderFileName.c_str(), material->pPixelShader, _device);

	return E_NOTIMPL;
}

std::string MeshLoader::getFullPath(const char * fileName) const {
	std::string fullPath(fileName);
	fullPath = _folderPath + fileName;

	return fullPath;
}

bool MeshLoader::isSkeletalMesh() {
	return _type == MeshType::Skeletal;
}

LocalMesh MeshLoader::getMeshes() const {
	return std::move(_mesh);
}

MeshType MeshLoader::getType() const {
	return _type;
}