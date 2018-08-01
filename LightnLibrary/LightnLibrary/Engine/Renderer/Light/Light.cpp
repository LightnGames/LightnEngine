#include "Light.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererSettings.h>
#include <Components/LightComponent.h>
#include <Components/CameraComponent.h>

Light::Light() {
}

Light::~Light() {
}

void Light::initializeLight(ComPtr<ID3D11Device>& device, const std::string& vertexShader, const std::string& pixelShader)
{
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	uint16 numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;


	RendererUtil::createVertexShader(vertexShader.c_str(), _vertexShader, polygonLayout, numElements, _layout, device);
	RendererUtil::createPixelShader(pixelShader.c_str(), _pixelShader, device);
	RendererUtil::createConstantBuffer(_matrixBuffer, sizeof(MeshConstantBuffer), device);

	_sampleState = GraphicsResourceManager::instance().simpleSamplerState();
}

void Light::draw(const DrawSettings & settings, RefPtr<LightComponent>& lightComponent)
{
	auto deviceContext = settings.deviceContext;
	auto deferredBuffers = settings.deferredBuffers.get();

	MeshConstantBuffer constantBuffer;

	auto& camera = CameraComponent::mainCamera;

	//�ˉe�ϊ��s����Z�b�g
	constantBuffer.mtxProj = Matrix4::transpose(Matrix4::multiply(camera->mtxProj(),camera->cameraMatrix()));

	//�J�����r���[�s����Z�b�g
	constantBuffer.mtxView = Matrix4::transpose(camera->cameraMatrix().inverse());

	//world�s����Z�b�g
	constantBuffer.mtxWorld = Matrix4::transpose(lightComponent->getWorldMatrix());

	//�J�������W���Z�b�g
	constantBuffer.cameraPos = Vector4(camera->getWorldPosition());

	//�g�p�V�F�[�_�[���Z�b�g
	deviceContext->VSSetShader(_vertexShader.Get(), NULL, 0);
	deviceContext->PSSetShader(_pixelShader.Get(), NULL, 0);

	//���_�C���v�b�g���C�A�E�g���Z�b�g
	deviceContext->IASetInputLayout(_layout.Get());

	//�R���X�^���g�o�b�t�@�[���e�X�V
	deviceContext->UpdateSubresource(_matrixBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);

	//�R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�ɃZ�b�g
	deviceContext->VSSetConstantBuffers(0, 1, _matrixBuffer.GetAddressOf());

	//GBuffer���Z�b�g
	deviceContext->PSSetShaderResources(0, 1, deferredBuffers->getShaderResourceView(0).GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getShaderResourceView(1).GetAddressOf());
	deviceContext->PSSetShaderResources(2, 1, deferredBuffers->getShaderResourceView(2).GetAddressOf());

	if (RendererSettings::skyBox.Get() != nullptr) {
		deviceContext->PSSetShaderResources(3, 1, RendererSettings::skyBox.GetAddressOf());
	}

	//�e�N�X�`���T���v���[���Z�b�g
	deviceContext->PSSetSamplers(0, 1, _sampleState.GetAddressOf());

	//�|���S���̕`�惋�[�����Z�b�g
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�`��
	deviceContext->Draw(4, 0);
}
