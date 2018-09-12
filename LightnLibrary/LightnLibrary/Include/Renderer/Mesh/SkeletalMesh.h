#pragma once

#include <Util/Util.h>

#include <d3d11.h>
#include <LMath.h>
#include <vector>

#include <Renderer/Mesh/Skeleton.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/Mesh/StaticMesh.h>

//頂点構造体
struct SKVertex : MeshVertex {

	UINT boneIndex[4];//ボーンインデックス
	float boneWeight[4];//ボーンウェイト

	SKVertex() :boneIndex{ 0,0,0,0 }, boneWeight{ 0.0f,0.0f,0.0f,0.0f } {
	}
};


//スケルタルメッシュ定数バッファ
struct SkeletalMeshConstantBuffer :MeshConstantBuffer {

	Matrix4 bone[MAX_BONES];

	SkeletalMeshConstantBuffer() {
		for (int i = 0; i<MAX_BONES; i++) {
			bone[i] = Matrix4::identity;
		}
	}

	SkeletalMeshConstantBuffer(const MeshConstantBuffer& meshBuffer) {
		memcpy(this, &meshBuffer, sizeof(MeshConstantBuffer));
	}
};

class SkeletalMesh :public StaticMesh {

public:

	SkeletalMesh(const LocalMesh& meshes, std::unique_ptr<Skeleton> skeleton);

	//メッシュを描画
	virtual void draw(const DrawSettings& drawSettings, const Matrix4& worldMatrix) override;

	virtual void drawDepth(const DrawSettings& drawSettings, const Matrix4& worldMatrix) override;

	//スケルトンデータを取得
	RefPtr<Skeleton> getSkeleton();

	RefPtr<Avator> avator();

	//現在再生中のアニメーションボーン行列をインデックスで取得
	Matrix4 getPlayingAnimPoseMatrix(int index) const;

private:

	//スケルトン
	std::unique_ptr<Avator> _avator;

};