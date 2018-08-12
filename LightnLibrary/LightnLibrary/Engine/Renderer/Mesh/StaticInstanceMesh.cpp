#include "StaticInstanceMesh.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererSettings.h>
#include <Renderer/StaticInstancedMeshRenderer.h>

StaticInstanceMesh::StaticInstanceMesh(const LocalMesh & meshes) :_meshes{ meshes }
{
}

void StaticInstanceMesh::setUp(
	ComPtr<ID3D11Device> device,
	const std::vector<Matrix4>& matrices,
	uint32 meshDrawOffset,
	uint32 matrixBufferOffset)
{

	_meshDrawOffset = meshDrawOffset;
	_matrixBufferOffset = matrixBufferOffset;

	//�J�����O�Ώۂ̃o�E���f�B���O�{�b�N�X�z��𐶐�
	std::vector<BoundingBoxInfo> boundingBoxes;
	{
		boundingBoxes.reserve(matrices.size());
		_cullingBoxes.reserve(matrices.size());

		for (auto&& m : matrices) {
			AABB transformdAABB = _meshes.boundingBox.createTransformMatrix(m);
			boundingBoxes.emplace_back(BoundingBoxInfo(transformdAABB.size(), Matrix4::transpose(m)));

			Vector3 p(m.m[3][0], m.m[3][1], m.m[3][2]);
			transformdAABB.translate(p);
			_cullingBoxes.emplace_back(transformdAABB);
		}

		//�C���X�^���X����ێ�
		_instanceCount = boundingBoxes.size();

		//GPU��4�X���b�h�œ��삷�邽�ߕs�������l�߂Ă���
		uint32 size = THREAD_NUM - (boundingBoxes.size() % THREAD_NUM);
		if (size < THREAD_NUM) {
			for (int i = 0; i < size; ++i) {
				boundingBoxes.emplace_back(BoundingBoxInfo(Vector3::zero, Matrix4::scaleXYZ(0, 0, 0)));
			}
		}

		//�s�������l�߂���̐�
		_paddingedCount = boundingBoxes.size();

		//0�ɑS�̃��b�V���̃I�t�Z�b�g 1�ɃX���b�h���̔{���ɍ��킹��O�̐� 2�Ƀ}�e���A����
		boundingBoxes[0].extent.w = meshDrawOffset;
		boundingBoxes[1].extent.w = _instanceCount;
		boundingBoxes[2].extent.w = _meshes.materialSlots.size(); 
	}

	//GPU�o�b�t�@�𐶐�
	{
		//�����ɓn���o�E���f�B���O�{�b�N�X���X�g�o�b�t�@
		D3D11_BUFFER_DESC cb;
		ZeroMemory(&cb, sizeof(cb));
		cb.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		cb.ByteWidth = _paddingedCount * sizeof(BoundingBoxInfo);
		cb.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		cb.StructureByteStride = sizeof(BoundingBoxInfo);

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = boundingBoxes.data();

		HRESULT hr = device->CreateBuffer(&cb, &initData, _boundingBoxListBuffer.ReleaseAndGetAddressOf());
		hr = device->CreateShaderResourceView(_boundingBoxListBuffer.Get(), nullptr, _boundingBoxListSRV.ReleaseAndGetAddressOf());

		const uint32 m[4] = { _matrixBufferOffset,0,0,0 };
		RendererUtil::createConstantBuffer(_meshDrawOffsetConstantBuffer, sizeof(uint32) * 4, device, m); 
	}
}

void StaticInstanceMesh::draw(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData)
{
	auto deviceContext = drawSettings.deviceContext;

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(Matrix4::identity, drawSettings.camera);

	//���_�o�b�t�@���Z�b�g
	const UINT stride = sizeof(MeshVertex);
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);
	deviceContext->VSSetShaderResources(0, 1, instanceData->instanceMatrixSRV.GetAddressOf());
	deviceContext->VSSetConstantBuffers(1, 1, _meshDrawOffsetConstantBuffer.GetAddressOf());

	//�|���S���̕`�惋�[�����Z�b�g
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�}�e���A���̐��������[�v
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];

		//�g�p�V�F�[�_�[���Z�b�g
		deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);
		deviceContext->PSSetShader(material->pPixelShader.Get(), NULL, 0);

		//�C���f�b�N�X�o�b�t�@���Z�b�g
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//���_�C���v�b�g���C�A�E�g���Z�b�g
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());

		//�R���X�^���g�o�b�t�@�[���e�X�V
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);

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
		deviceContext->DrawIndexedInstancedIndirect(instanceData->indtsnceDrawListBuffer.Get(), (_meshDrawOffset + j) * 20);

	}
}


#include <Renderer/SceneRendererManager.h>
void StaticInstanceMesh::drawDepth(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData)
{
	auto deviceContext = drawSettings.deviceContext;

	for (auto&& b : _cullingBoxes) {
		//SceneRendererManager::debugDrawBox(b.center(), b.size());
	}

	//�J�����O
	{
		uint32 offset = _matrixBufferOffset;
		deviceContext->CSSetShaderResources(0, 1, _boundingBoxListSRV.GetAddressOf());
		deviceContext->CSSetConstantBuffers(0, 1, instanceData->frustumCullingBuffer.GetAddressOf());
		deviceContext->CSSetUnorderedAccessViews(0, 1, instanceData->instanceMatrixUAV.GetAddressOf(), &offset);
		deviceContext->CSSetUnorderedAccessViews(1, 1, instanceData->instanceDrawListUAV.GetAddressOf(), 0);
		deviceContext->CSSetShader(instanceData->cullingShader.Get(), nullptr, 0);
		deviceContext->Dispatch(_paddingedCount / THREAD_NUM, 1, 1);

		ID3D11UnorderedAccessView* ppUAViewNULL[2] = { nullptr, nullptr };
		ID3D11ShaderResourceView* ppSRVNULL[1] = { nullptr };
		ID3D11Buffer* ppCBNULL[1] = { nullptr };
		deviceContext->CSSetShader(nullptr, nullptr, 0);
		deviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);
		deviceContext->CSSetUnorderedAccessViews(0, 2, ppUAViewNULL, nullptr);
		deviceContext->CSSetShaderResources(0, 1, ppSRVNULL);
	}

	//�`��
	{
		MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(Matrix4::identity, drawSettings.camera);

		//���_�o�b�t�@���Z�b�g
		const UINT stride = sizeof(MeshVertex);
		const UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);
		deviceContext->VSSetShaderResources(0, 1, instanceData->instanceMatrixSRV.GetAddressOf());
		deviceContext->VSSetConstantBuffers(1, 1, _meshDrawOffsetConstantBuffer.GetAddressOf());

		//�|���S���̕`�惋�[�����Z�b�g
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�}�e���A���̐��������[�v
		for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

			const auto& material = _meshes.materialSlots[j];
			deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);
			deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->IASetInputLayout(material->pVertexLayout.Get());
			deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);
			deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());
			deviceContext->DrawIndexedInstancedIndirect(instanceData->indtsnceDrawListBuffer.Get(), (_meshDrawOffset + j) * 20);
		}
	}
}

RefPtr<const LocalMesh> StaticInstanceMesh::meshInfo() const{
	return &_meshes;
}

RefPtr<MaterialData> StaticInstanceMesh::material(int index) const
{
	return _meshes.materialSlots[index].get();
}
