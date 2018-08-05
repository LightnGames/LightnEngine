#include "TileBasedLightCulling.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/RendererSettings.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Components/CameraComponent.h>
#include <ShaderDefines.h>

struct TileBasedPointLightType {
	Vector3 positionView;
	float attenuationBegin;
	Vector3 color;
	float attenuationEnd;
};

struct PerFrameConstants{
	Matrix4 mCameraWorldViewProj;
	Matrix4 mCameraWorldView;
	Matrix4 mCameraViewProj;
	Matrix4 mCameraProj;
	Vector4 mCameraNearFar;
	uint32 mFramebufferDimensionsX;
	uint32 mFramebufferDimensionsY;
	uint32 mFramebufferDimensionsZ;
	uint32 mFramebufferDimensionsW;
};

struct PerTilePlane {
	Vector4 right;
	Vector4 left;
	Vector4 up;
	Vector4 down;
};

// Flat framebuffer RGBA16-encoded
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

	//ポイントライトリストシェーダーリソースビュー
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	SRVDesc.BufferEx.FirstElement = 0;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.BufferEx.NumElements = MAX_LIGHTS;

	hr = device->CreateShaderResourceView(_pointLightListBuffer.Get(), &SRVDesc, _pointLightListSRV.ReleaseAndGetAddressOf());

	uint32 dispatchWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	uint32 dispatchHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;

	//ライトフラスタム
	D3D11_BUFFER_DESC cbP;
	ZeroMemory(&cbP, sizeof(cbP));
	cbP.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	cbP.ByteWidth = sizeof(PerTilePlane)*dispatchWidth*dispatchHeight;
	cbP.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	cbP.StructureByteStride = sizeof(PerTilePlane);

	hr = device->CreateBuffer(&cbP, 0, _perTilePlaneBuffer.ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(_perTilePlaneBuffer.Get(), 0, _perTilePlaneSRV.ReleaseAndGetAddressOf());


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
	hr = RendererUtil::createComputeShader("CoputeDeferredTile.cso", _tileBasedLightCullingShader, device);
	hr = RendererUtil::createPixelShader("StructuredToRenderTarget.cso", _structuredToRenderShader, device);

	return S_OK;
}


#include <Renderer/SceneRendererManager.h>
void TileBasedLightCulling::draw(const DrawSettings& settings) {

	auto deferredBuffers = settings.deferredBuffers;
	auto deviceContext = settings.deviceContext;
	auto& camera = CameraComponent::mainCamera;

	ID3D11ShaderResourceView* resource[4] = {
	deferredBuffers->getDepthStencilResource().Get(),
	deferredBuffers->getShaderResourceView(0).Get(),
	deferredBuffers->getShaderResourceView(1).Get(),
	deferredBuffers->getShaderResourceView(2).Get(),
	};

	std::vector<TileBasedPointLightType> pointLights;
	pointLights.reserve(MAX_LIGHTS);

	uint32 width = deferredBuffers->getGBufferSize().x;
	uint32 height = deferredBuffers->getGBufferSize().y;
	const uint32 dispatchWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	const uint32 dispatchHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;


	TileBasedPointLightType p;
	p.positionView = Vector3(0,1.1f,1);
	p.attenuationBegin = 1;
	p.attenuationEnd = 10;
	p.color = Vector3(1, 1, 1)*20;
	SceneRendererManager::debugDrawSphere(p.positionView, p.attenuationEnd);
	p.positionView = Matrix4::transform(p.positionView, camera->cameraMatrix().inverse());
	pointLights.emplace_back(p);

	TileBasedPointLightType p2;
	p2.positionView = Vector3(8, 0.1f, -5.0f);
	p2.attenuationBegin = 1;
	p2.attenuationEnd = 10;
	p2.color = Vector3(1, 1, 1);
	SceneRendererManager::debugDrawSphere(p2.positionView, p2.attenuationEnd);
	p2.positionView = Matrix4::transform(p2.positionView, camera->cameraMatrix().inverse());
	pointLights.emplace_back(p2);

	PerFrameConstants perFrame;
	perFrame.mCameraWorldView = camera->cameraMatrix().transpose();
	perFrame.mCameraProj = camera->mtxProj().transpose();

	perFrame.mCameraViewProj =Matrix4::matrixFromQuaternion(camera->getWorldRotation()).transpose();
	perFrame.mCameraWorldViewProj = perFrame.mCameraProj.inverse();
	perFrame.mCameraNearFar = Vector4(camera->farClip(), camera->nearClip(), 0.0f, 0.0f);
	perFrame.mFramebufferDimensionsX = width;
	perFrame.mFramebufferDimensionsY = height;
	perFrame.mFramebufferDimensionsZ = 0;//ダミー
	perFrame.mFramebufferDimensionsW = 0;//ダミー

	auto sampler = GraphicsResourceManager::instance().simpleSamplerState();
	deviceContext->UpdateSubresource(_perFrameConstantBuffer.Get(), 0, 0, &perFrame, 0, 0);
	deviceContext->UpdateSubresource(_pointLightListBuffer.Get(), 0, 0, pointLights.data(), 0, 0);

	deviceContext->CSSetShader(_tileBasedLightCullingShader.Get(), 0, 0);
	deviceContext->CSSetShaderResources(0, 4, resource);
	deviceContext->CSSetShaderResources(4, 1, _pointLightListSRV.GetAddressOf());

	if (RendererSettings::skyBox.Get() != nullptr) {
		deviceContext->CSSetShaderResources(5, 1, RendererSettings::skyBox.GetAddressOf());
	}

	deviceContext->CSSetConstantBuffers(0, 1, _perFrameConstantBuffer.GetAddressOf());
	deviceContext->CSSetUnorderedAccessViews(0, 1, _frameBufferUAV.GetAddressOf(), 0);
	deviceContext->CSSetSamplers(0, 1, sampler.GetAddressOf());
	deviceContext->Dispatch(dispatchWidth, dispatchHeight, 1);

	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { 0 };
	ID3D11ShaderResourceView* ppSRVNULL[5] = { 0,0,0,0,0 };
	ID3D11Buffer* ppCBNULL[1] = { 0 };
	deviceContext->CSSetShader(0, 0, 0);
	deviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, 0);
	deviceContext->CSSetShaderResources(0, 5, ppSRVNULL);

	deviceContext->PSSetShader(_structuredToRenderShader.Get(), 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _perFrameConstantBuffer.GetAddressOf());
	deviceContext->PSSetShaderResources(0, 1, _frameBufferSRV.GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getDepthStencilResource().GetAddressOf());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->Draw(4, 0);
}
