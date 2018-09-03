#include "Deferredbuffers.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/GraphicsBuffers.h>

Deferredbuffers::Deferredbuffers() :_renderTargets{ nullptr } {

	_depthStencilBuffer._pInterface = 0;
	_depthStencilView._pInterface = 0;
}

Deferredbuffers::~Deferredbuffers() {
}

HRESULT Deferredbuffers::initialize(ComPtr<ID3D11Device>& device, uint16 width, uint16 height, float screenDepth, float screenNear) {

	HRESULT result;
	D3D11_TEXTURE2D_DESC depthDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC stencilDesc;

	_width = width;
	_height = height;

	//アルベド
	createRenderTarget(0, DXGI_FORMAT_R8G8B8A8_UNORM, device);
	
	//ワールド法線
	createRenderTarget(1, DXGI_FORMAT_R10G10B10A2_UNORM, device);

	//ラフネス・メタルネス
	createRenderTarget(2, DXGI_FORMAT_R8G8B8A8_UNORM, device);

	//エミッシブ・ライティング結果用 64bit-HDR
	createRenderTarget(3, DXGI_FORMAT_R16G16B16A16_FLOAT, device);

	//デプステクスチャの生成
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = _width;
	depthDesc.Height = _height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&depthDesc, NULL, _depthStencilBuffer.ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//デプスステンシルビューの生成
	ZeroMemory(&stencilDesc, sizeof(stencilDesc));
	stencilDesc.Flags = 0;
	stencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	stencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	stencilDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(_depthStencilBuffer.Get(), &stencilDesc, _depthStencilView.ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//読み取り専用デプスステンシルビュー
	stencilDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
	result = device->CreateDepthStencilView(_depthStencilBuffer.Get(), &stencilDesc, _depthStencilViewReadOnly.ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(_depthStencilBuffer.Get(), &shaderDesc, _depthStencilSRV.ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//GBuffer用のデプステストステート
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;

	result = device->CreateDepthStencilState(&dsDesc, _depthStencilState.ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//ビューポートを設定
	_viewport.Width = static_cast<float>(_width);
	_viewport.Height = static_cast<float>(_height);
	_viewport.MinDepth = 0.0f;
	_viewport.MaxDepth = 1.0f;
	_viewport.TopLeftX = 0.0f;
	_viewport.TopLeftY = 0.0f;

	return S_OK;
}

void Deferredbuffers::cleanUp() {

	if (_depthStencilView != NULL) {
		_depthStencilView->Release();
		_depthStencilView._pInterface = 0;
	}

	if (_depthStencilBuffer != NULL) {
		_depthStencilBuffer->Release();
		_depthStencilBuffer._pInterface = 0;
	}
}

void Deferredbuffers::setRenderTargets(ComPtr<ID3D11DeviceContext> context) {

	ID3D11RenderTargetView* rtvs[BUFFER_COUNT] = { 0 };
	for (int i = 0; i < BUFFER_COUNT; ++i) {
		rtvs[i] = _renderTargets[i]->rtv();
	}
	
	context->OMSetRenderTargets(BUFFER_COUNT, rtvs, _depthStencilView.Get());
	context->RSSetViewports(1, &_viewport);
	context->OMSetBlendState(0, 0, D3D11_DEFAULT_SAMPLE_MASK);
	context->OMSetDepthStencilState(_depthStencilState.Get(), 1);
}

void Deferredbuffers::setRenderTargetLighting(ComPtr<ID3D11DeviceContext> deviceContext) {
	deviceContext->OMSetRenderTargets(1, _renderTargets[3]->ppRtv(), 0);
}

void Deferredbuffers::setRenderTargetEaryZ(ComPtr<ID3D11DeviceContext> context)
{
	context->OMSetRenderTargets(0, 0, _depthStencilView.Get());
	context->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetDepthStencilState(0, 0);
	context->PSSetShader(0, 0, 0);
}

void Deferredbuffers::clearRenderTargets(ComPtr<ID3D11DeviceContext> context, float red, float green, float blue, float alpha) {
	
	const float color[4] = { red,green,blue,alpha };

	for (uint16 i = 0; i < BUFFER_COUNT; ++i) {
		context->ClearRenderTargetView(_renderTargets[i]->rtv(), color);
	}
}

void Deferredbuffers::setViewPort(ComPtr<ID3D11DeviceContext> deviceContext) {
	deviceContext->RSSetViewports(1, &_viewport);
}

ID3D11ShaderResourceView * Deferredbuffers::getShaderResourceView(uint16 index) const {
	return _renderTargets[index]->srv();
}

RefPtr<RenderTarget> Deferredbuffers::getRenderTarget(uint16 index) {
	return _renderTargets[index].get();
}

ComPtr<ID3D11ShaderResourceView> Deferredbuffers::getDepthStencilResource() const
{
	return _depthStencilSRV;
}

ID3D11DepthStencilView* Deferredbuffers::getDepthStencilView(bool readOnly) {
	return readOnly ? _depthStencilViewReadOnly.Get() : _depthStencilView.Get();
}

Vector2 Deferredbuffers::getGBufferSize() const {
	return Vector2(_width, _height);
}

HRESULT Deferredbuffers::createRenderTarget(uint16 index, DXGI_FORMAT format, ComPtr<ID3D11Device>& device) {
	
	_renderTargets[index] = std::make_unique<RenderTarget>(
		device.Get(),
		format,
		_width,
		_height);

	return S_OK;
}
