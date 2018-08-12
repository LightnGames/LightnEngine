#include "SkeletalMesh.h"
#include <Renderer/RendererSettings.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/SceneRendererManager.h>

SkeletalMesh::SkeletalMesh(const LocalMesh& meshes, std::unique_ptr<Skeleton> skeleton)
	:StaticMesh(meshes), _avator{ std::make_unique<Avator>( std::move(skeleton)) } {
}

void SkeletalMesh::draw(const DrawSettings& drawSettings, const Matrix4& worldMatrix) {

	const ComPtr<ID3D11DeviceContext>& deviceContext = drawSettings.deviceContext;

	SkeletalMeshConstantBuffer constantBuffer(RendererUtil::getConstantBuffer(worldMatrix, drawSettings.camera));


	//�{�[���s����Z�b�g
	for (UINT k = 0; k < _avator->getSize(); ++k) {
		const Matrix4 mat = getPlayingAnimPoseMatrix(k);
		constantBuffer.bone[k] = Matrix4::transpose(mat);
		//constantBuffer.bone[k] = Matrix4::identity;
	};

	drawMesh(deviceContext, static_cast<void*>(&constantBuffer), sizeof(SKVertex));

	return;

	for (UINT j = 0; j < _avator->getSize(); ++j) {
		Matrix4 boneMtx = getPlayingAnimPoseMatrix(j).multiply(worldMatrix);

		Vector3 start = Vector3::zero;
		Vector3 end = Vector3(boneMtx.m[3][0], boneMtx.m[3][1], boneMtx.m[3][2]);
		SceneRendererManager::debugDrawLine(start, end);
	}

}

void SkeletalMesh::drawDepth(const DrawSettings & drawSettings, const Matrix4 & worldMatrix)
{
	auto deviceContext = drawSettings.deviceContext;
	SkeletalMeshConstantBuffer constantBuffer(RendererUtil::getConstantBuffer(worldMatrix, drawSettings.camera));

	//�{�[���s����Z�b�g
	for (UINT k = 0; k < _avator->getSize(); ++k) {
		const Matrix4 mat = getPlayingAnimPoseMatrix(k);
		constantBuffer.bone[k] = Matrix4::transpose(mat);
		//constantBuffer.bone[k] = Matrix4::identity;
	};

	//���_�o�b�t�@���Z�b�g
	const UINT stride = sizeof(SKVertex);
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);

	//�}�e���A���̐��������[�v
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];

		//�g�p�V�F�[�_�[���Z�b�g
		deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);

		//�C���f�b�N�X�o�b�t�@���Z�b�g
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//���_�C���v�b�g���C�A�E�g���Z�b�g
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());

		//�|���S���̕`�惋�[�����Z�b�g
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�R���X�^���g�o�b�t�@�[���e�X�V
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);

		//�R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�ɃZ�b�g
		deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());

		//�`��
		deviceContext->DrawIndexed(material->faceCount * 3, 0, 0);

	}
}

RefPtr<Skeleton> SkeletalMesh::getSkeleton() {
	return _avator->bindPose.get();
}

RefPtr<Avator> SkeletalMesh::avator() {
	return _avator.get();
}

const Matrix4 & SkeletalMesh::getPlayingAnimPoseMatrix(int index) const {

	if (_avator->animatedPose != nullptr) {
		const Matrix4 inv = Matrix4::inverse((*_avator->bindPose)[index].matrix);//FBX�̃o�C���h�|�[�Y�͏����p���i��΍��W�j
		const Matrix4 ret = Matrix4::multiply(inv, (*_avator->animatedPose)[index].matrix);//�o�C���h�|�[�Y�̋t�s��ƃt���[���p���s���������B
		return ret;
	}

	return Matrix4::identity;
}
