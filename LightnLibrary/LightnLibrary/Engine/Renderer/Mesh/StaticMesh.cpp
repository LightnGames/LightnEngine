#include "StaticMesh.h"
#include <Renderer/RendererSettings.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/SceneRendererManager.h>

StaticMesh::StaticMesh(const LocalMesh& meshes) :_meshes{ meshes } {
}

void StaticMesh::setUp(ComPtr<ID3D11Device> device) {
	
}

void StaticMesh::draw(const DrawSettings& drawSettings, const Matrix4& worldMatrix) {

	auto deviceContext = drawSettings.deviceContext;

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(worldMatrix, drawSettings.camera);

	drawMesh(deviceContext, static_cast<void*>(&constantBuffer), sizeof(MeshVertex));
}

void StaticMesh::drawDepth(const DrawSettings & drawSettings, const Matrix4 & worldMatrix)
{
	ComPtr<ID3D11DeviceContext> deviceContext = drawSettings.deviceContext;

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(worldMatrix, drawSettings.camera);

	//���_�o�b�t�@���Z�b�g
	const UINT stride = sizeof(MeshVertex);
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);

	//�}�e���A���̐��������[�v
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];
		deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());
		deviceContext->DrawIndexed(material->faceCount * 3, 0, 0);
	}
}

void StaticMesh::drawMesh(ComPtr<ID3D11DeviceContext> deviceContext, const RefPtr<void>& constantBuffer, const UINT vertexBufferSize) {

	//���_�o�b�t�@���Z�b�g
	const UINT stride = vertexBufferSize;
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);
	
	//�|���S���̕`�惋�[�����Z�b�g
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�}�e���A���̐��������[�v
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];

		//�g�p�V�F�[�_�[���Z�b�g
		deviceContext->VSSetShader(material->pVertexShader.Get(), 0, 0);
		deviceContext->PSSetShader(material->pPixelShader.Get(), 0, 0);

		//�C���f�b�N�X�o�b�t�@���Z�b�g
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//���_�C���v�b�g���C�A�E�g���Z�b�g
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());

		//�R���X�^���g�o�b�t�@�[���e�X�V
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, constantBuffer.get(), 0, 0);

		//�R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�ɃZ�b�g
		deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());

		//CubeMap�e�N�X�`�����Z�b�g
		int globalTextureCount = 1;
		if (RendererSettings::skyBox.Get() != nullptr) {
			deviceContext->PSSetShaderResources(0, 1, RendererSettings::skyBox.GetAddressOf());
		}

		//�e�N�X�`�����\�[�X���Z�b�g(�V�F�[�_�[���̕��я��ƈ�v����K�v����)
		for (UINT k = 0; k < material->textureCount; ++k) {
			deviceContext->PSSetShaderResources(k + globalTextureCount, 1, material->ppTextures[k].GetAddressOf());
		}

		//�e�N�X�`���T���v���[���Z�b�g
		deviceContext->PSSetSamplers(0, 1, material->pSamplerLiner.GetAddressOf());

		//�`��
		deviceContext->DrawIndexed(material->faceCount * 3, 0, 0);

	}
}

RefPtr<MaterialData> StaticMesh::material(int index) {
	return _meshes.materialSlots[index].get();
}
