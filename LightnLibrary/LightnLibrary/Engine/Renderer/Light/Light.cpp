#include "Light.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/GameRenderer.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererSettings.h>
#include <Components/LightComponent.h>

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

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(lightComponent->getWorldMatrix(), settings.camera);

	auto& camera = settings.camera;

	//使用シェーダーをセット
	deviceContext->VSSetShader(_vertexShader.Get(), 0, 0);
	deviceContext->PSSetShader(_pixelShader.Get(), 0, 0);

	//頂点インプットレイアウトをセット
	deviceContext->IASetInputLayout(_layout.Get());

	//コンスタントバッファー内容更新
	deviceContext->UpdateSubresource(_matrixBuffer.Get(), 0, 0, &constantBuffer, 0, 0);

	//コンスタントバッファーを使うシェーダーにセット
	deviceContext->VSSetConstantBuffers(0, 1, _matrixBuffer.GetAddressOf());

	ID3D11ShaderResourceView* const srvs[4] = {
		deferredBuffers->getDepthStencilResource().Get(),
		deferredBuffers->getShaderResourceView(0),
		deferredBuffers->getShaderResourceView(1),
		deferredBuffers->getShaderResourceView(2),
	};

	//GBufferをセット
	deviceContext->PSSetShaderResources(0, 4, srvs);

	//テクスチャサンプラーをセット
	deviceContext->PSSetSamplers(0, 1, _sampleState.GetAddressOf());

	//ポリゴンの描画ルールをセット
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//描画
	deviceContext->Draw(4, 0);
}

ShadowResource Light::createShadow(uint32 size) {
	ShadowResource shadow(size);
	shadow.initialize(GameRenderer::instance().device());
	return std::move(shadow);
}

HRESULT ShadowResource::initialize(ComPtr<ID3D11Device> device) {
	HRESULT hr;
	D3D11_TEXTURE2D_DESC depthDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC stencilDesc;

	//デプステクスチャの生成
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = shadowMapWidth;
	depthDesc.Height = shadowMapHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	hr = device->CreateTexture2D(&depthDesc, NULL, _depthStencilBuffer.ReleaseAndGetAddressOf());

	//デプスステンシルビューの生成
	ZeroMemory(&stencilDesc, sizeof(stencilDesc));
	stencilDesc.Flags = 0;
	stencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	stencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	stencilDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(_depthStencilBuffer.Get(), &stencilDesc, _depthStencilView.ReleaseAndGetAddressOf());

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(_depthStencilBuffer.Get(), &shaderDesc, _depthStencilSRV.ReleaseAndGetAddressOf());

	shadowVp.Width = static_cast<float>(shadowMapWidth);
	shadowVp.Height = static_cast<float>(shadowMapHeight);
	shadowVp.MinDepth = 0.0f;
	shadowVp.MaxDepth = 1.0f;
	shadowVp.TopLeftX = 0.0f;
	shadowVp.TopLeftY = 0.0f;

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 1.0f;
	desc.BorderColor[2] = 1.0f;
	desc.BorderColor[3] = 1.0f;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	desc.MaxAnisotropy = 1;
	desc.MipLODBias = 0;
	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = +FLT_MAX;

	// サンプラーステートを生成.
	hr = device->CreateSamplerState(&desc, _shadowSampler.ReleaseAndGetAddressOf());

	return S_OK;
}
