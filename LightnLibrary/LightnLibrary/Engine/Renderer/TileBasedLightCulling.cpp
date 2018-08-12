#include "TileBasedLightCulling.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/RendererSettings.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Components/CameraComponent.h>
#include <ShaderDefines.h>

struct PerFrameConstants{
	Matrix4 camerProjInverse;
	Matrix4 cameraRotate;
	Matrix4 cameraProj;
	Vector2 cameraNearFar;
	uint32 framebufferDimensionsX;
	uint32 framebufferDimensionsY;
};

//RGBAを16bitごとに分ける
struct FramebufferFlatElement {
	uint32 rb;
	uint32 ga;
};

TileBasedLightCulling::TileBasedLightCulling() {
}

TileBasedLightCulling::~TileBasedLightCulling() {
}

HRESULT TileBasedLightCulling::initialize(ComPtr<ID3D11Device> device, uint32 width, uint32 height) {

	HRESULT hr;

	//ポイントライトリスト
	D3D11_BUFFER_DESC cb;
	ZeroMemory(&cb, sizeof(cb));
	cb.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	cb.ByteWidth = sizeof(TileBasedPointLightType)*MAX_LIGHTS;
	cb.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	cb.StructureByteStride = sizeof(TileBasedPointLightType);

	hr = device->CreateBuffer(&cb, 0, _pointLightListBuffer.ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(_pointLightListBuffer.Get(), nullptr, _pointLightListSRV.ReleaseAndGetAddressOf());

	//スポットライトリスト
	cb.ByteWidth = sizeof(TileBasedSpotLightType)*MAX_LIGHTS;
	cb.StructureByteStride = sizeof(TileBasedSpotLightType);

	hr = device->CreateBuffer(&cb, 0, _spotLightListBuffer.ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(_spotLightListBuffer.Get(), nullptr, _spotLightListSRV.ReleaseAndGetAddressOf());

	//レンダーターゲット代わりのバッファ
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(FramebufferFlatElement)*width*height;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(FramebufferFlatElement);

	hr = device->CreateBuffer(&desc, 0, _frameBuffer.ReleaseAndGetAddressOf());
	hr = device->CreateUnorderedAccessView(_frameBuffer.Get(), 0, _frameBufferUAV.ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(_frameBuffer.Get(), 0, _frameBufferSRV.ReleaseAndGetAddressOf());

	hr = RendererUtil::createConstantBuffer(_perFrameConstantBuffer, sizeof(PerFrameConstants), device);
	hr = RendererUtil::createComputeShader("ComputeDeferredTile.cso", _tileBasedLightCullingShader, device);
	hr = RendererUtil::createPixelShader("StructuredToRenderTarget.cso", _structuredToRenderShader, device);

	return S_OK;
}


void TileBasedLightCulling::draw(const DrawSettings& settings,
	const TileBasedPointLightType* pointLights,
	const TileBasedSpotLightType* spotLights) {

	auto deferredBuffers = settings.deferredBuffers;
	auto deviceContext = settings.deviceContext;
	auto& camera = CameraComponent::mainCamera;

	ID3D11ShaderResourceView* resource[4] = {
	deferredBuffers->getDepthStencilResource().Get(),
	deferredBuffers->getShaderResourceView(0).Get(),
	deferredBuffers->getShaderResourceView(1).Get(),
	deferredBuffers->getShaderResourceView(2).Get(),
	};

	const uint32 width = deferredBuffers->getGBufferSize().x;
	const uint32 height = deferredBuffers->getGBufferSize().y;
	const uint32 dispatchWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	const uint32 dispatchHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;

	PerFrameConstants perFrame;
	perFrame.cameraProj = camera->mtxProj().transpose();
	perFrame.cameraRotate =Matrix4::matrixFromQuaternion(camera->getWorldRotation()).transpose();
	perFrame.camerProjInverse = perFrame.cameraProj.inverse();
	perFrame.cameraNearFar = Vector2(camera->farClip(), camera->nearClip());
	perFrame.framebufferDimensionsX = width;
	perFrame.framebufferDimensionsY = height;
	
	auto sampler = GraphicsResourceManager::instance().simpleSamplerState();
	deviceContext->UpdateSubresource(_perFrameConstantBuffer.Get(), 0, 0, &perFrame, 0, 0);
	deviceContext->UpdateSubresource(_pointLightListBuffer.Get(), 0, 0, pointLights, 0, 0);
	deviceContext->UpdateSubresource(_spotLightListBuffer.Get(), 0, 0, spotLights, 0, 0);

	deviceContext->CSSetShader(_tileBasedLightCullingShader.Get(), 0, 0);
	deviceContext->CSSetShaderResources(0, 4, resource);
	deviceContext->CSSetShaderResources(4, 1, _pointLightListSRV.GetAddressOf());
	deviceContext->CSSetShaderResources(5, 1, _spotLightListSRV.GetAddressOf());

	if (RendererSettings::skyBox.Get() != nullptr) {
		deviceContext->CSSetShaderResources(6, 1, RendererSettings::skyBox.GetAddressOf());
	}

	deviceContext->CSSetConstantBuffers(0, 1, _perFrameConstantBuffer.GetAddressOf());
	deviceContext->CSSetUnorderedAccessViews(0, 1, _frameBufferUAV.GetAddressOf(), 0);
	deviceContext->CSSetSamplers(0, 1, sampler.GetAddressOf());
	deviceContext->Dispatch(dispatchWidth, dispatchHeight, 1);

	//ComputeShaderリセット　特にSRVはリセットしないとUAVがAPIのセーフガードで抹消される
	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { 0 };
	ID3D11ShaderResourceView* ppSRVNULL[5] = { 0,0,0,0,0 };
	ID3D11Buffer* ppCBNULL[1] = { 0 };
	deviceContext->CSSetShader(0, 0, 0);
	deviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, 0);
	deviceContext->CSSetShaderResources(0, 5, ppSRVNULL);

	//StructuredからRenderTargetに描画
	deviceContext->PSSetShader(_structuredToRenderShader.Get(), 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _perFrameConstantBuffer.GetAddressOf());
	deviceContext->PSSetShaderResources(0, 1, _frameBufferSRV.GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getDepthStencilResource().GetAddressOf());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->Draw(4, 0);

	deviceContext->PSSetShaderResources(0, 2, ppSRVNULL);
}
