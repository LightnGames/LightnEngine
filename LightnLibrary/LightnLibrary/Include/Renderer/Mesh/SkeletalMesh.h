#pragma once

#include <Util/Util.h>

#include <d3d11.h>
#include <LMath.h>
#include <vector>

#include <Renderer/Mesh/Skeleton.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/Mesh/StaticMesh.h>

//���_�\����
struct SKVertex : MeshVertex {

	UINT boneIndex[4];//�{�[���C���f�b�N�X
	float boneWeight[4];//�{�[���E�F�C�g

	SKVertex() :boneIndex{ 0,0,0,0 }, boneWeight{ 0.0f,0.0f,0.0f,0.0f } {
	}
};


//�X�P���^�����b�V���萔�o�b�t�@
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

	//���b�V����`��
	virtual void draw(const DrawSettings& drawSettings, const Matrix4& worldMatrix) override;

	virtual void drawDepth(const DrawSettings& drawSettings, const Matrix4& worldMatrix) override;

	//�X�P���g���f�[�^���擾
	RefPtr<Skeleton> getSkeleton();

	RefPtr<Avator> avator();

	//���ݍĐ����̃A�j���[�V�����{�[���s����C���f�b�N�X�Ŏ擾
	Matrix4 getPlayingAnimPoseMatrix(int index) const;

private:

	//�X�P���g��
	std::unique_ptr<Avator> _avator;

};