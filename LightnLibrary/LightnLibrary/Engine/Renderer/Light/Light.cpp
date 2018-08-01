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

	//射影変換行列をセット
	constantBuffer.mtxProj = Matrix4::transpose(Matrix4::multiply(camera->mtxProj(),camera->cameraMatrix()));

	//カメラビュー行列をセット
	constantBuffer.mtxView = Matrix4::transpose(camera->cameraMatrix().inverse());

	//world行列をセット
	constantBuffer.mtxWorld = Matrix4::transpose(lightComponent->getWorldMatrix());

	//カメラ座標をセット
	constantBuffer.cameraPos = Vector4(camera->getWorldPosition());

	//使用シェーダーをセット
	deviceContext->VSSetShader(_vertexShader.Get(), NULL, 0);
	deviceContext->PSSetShader(_pixelShader.Get(), NULL, 0);

	//頂点インプットレイアウトをセット
	deviceContext->IASetInputLayout(_layout.Get());

	//コンスタントバッファー内容更新
	deviceContext->UpdateSubresource(_matrixBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);

	//コンスタントバッファーを使うシェーダーにセット
	deviceContext->VSSetConstantBuffers(0, 1, _matrixBuffer.GetAddressOf());

	//GBufferをセット
	deviceContext->PSSetShaderResources(0, 1, deferredBuffers->getShaderResourceView(0).GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getShaderResourceView(1).GetAddressOf());
	deviceContext->PSSetShaderResources(2, 1, deferredBuffers->getShaderResourceView(2).GetAddressOf());

	if (RendererSettings::skyBox.Get() != nullptr) {
		deviceContext->PSSetShaderResources(3, 1, RendererSettings::skyBox.GetAddressOf());
	}

	//テクスチャサンプラーをセット
	deviceContext->PSSetSamplers(0, 1, _sampleState.GetAddressOf());

	//ポリゴンの描画ルールをセット
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//描画
	deviceContext->Draw(4, 0);
}
