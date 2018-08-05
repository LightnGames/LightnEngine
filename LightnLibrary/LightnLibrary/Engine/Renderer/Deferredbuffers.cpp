#include "Deferredbuffers.h"
#include <Renderer/RendererUtil.h>

Deferredbuffers::Deferredbuffers() {

	for (int32 i = 0; i < BUFFER_COUNT; ++i) {
		_renderTargetTextureArray[i]._pInterface = 0;
		_renderTargetViewArray[i]._pInterface = 0;
		_shaderResourceViewArray[i]._pInterface = 0;
	}

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

	//カラーバッファ
	createRenderTarget(0, DXGI_FORMAT_R8G8B8A8_UNORM, device);
	
	//ワールド法線
	createRenderTarget(1, DXGI_FORMAT_R32G32B32A32_FLOAT, device);

	//ラフネス・メタルネス・エミッシブ
	createRenderTarget(2, DXGI_FORMAT_R8G8B8A8_UNORM, device);

	//デプステクスチャの生成
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = _width;
	depthDesc.Height = _height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
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
	stencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	stencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	stencilDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(_depthStencilBuffer.Get(), &stencilDesc, _depthStencilView.ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(_depthStencilBuffer.Get(), &shaderDesc, _depthStencilResource.ReleaseAndGetAddressOf());
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

	//ZPrePass用シェーダー作成
	RendererUtil::createPixelShader("Depth_ps.cso", _depthShader, device);

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

	for (int i = 0; i < BUFFER_COUNT; ++i) {

		if (_shaderResourceViewArray[i] != NULL) {
			_shaderResourceViewArray[i]->Release();
			_shaderResourceViewArray[i]._pInterface = 0;
		}

		if (_renderTargetViewArray[i] != NULL) {
			_renderTargetViewArray[i]->Release();
			_renderTargetViewArray[i]._pInterface = 0;
		}

		if (_renderTargetTextureArray[i] != NULL) {
			_renderTargetTextureArray[i]->Release();
			_renderTargetTextureArray[i]._pInterface = 0;
		}
	}
}

void Deferredbuffers::setRenderTargets(ComPtr<ID3D11DeviceContext> context) {
	context->OMSetRenderTargets(BUFFER_COUNT, _renderTargetViewArray[0].GetAddressOf(), _depthStencilView.Get());
	context->RSSetViewports(1, &_viewport);
	context->OMSetBlendState(NULL, NULL, 0xffffffff);
	context->OMSetDepthStencilState(_depthStencilState.Get(), 1);
}

void Deferredbuffers::setRenderTargetEaryZ(ComPtr<ID3D11DeviceContext> context)
{
	context->OMSetRenderTargets(0, NULL, _depthStencilView.Get());
	context->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->OMSetDepthStencilState(NULL, 0);
	context->PSSetShader(_depthShader.Get(), NULL, 0);
}

void Deferredbuffers::clearRenderTargets(ComPtr<ID3D11DeviceContext> context, float red, float green, float blue, float alpha) {
	
	const float color[4] = { red,green,blue,alpha };

	for (uint16 i = 0; i < BUFFER_COUNT; ++i) {
		context->ClearRenderTargetView(_renderTargetViewArray[i].Get(), color);
	}
}

ComPtr<ID3D11ShaderResourceView> Deferredbuffers::getShaderResourceView(uint16 index) const{
	return _shaderResourceViewArray[index];
}

ComPtr<ID3D11ShaderResourceView> Deferredbuffers::getDepthStencilResource() const
{
	return _depthStencilResource;
}

Vector2 Deferredbuffers::getGBufferSize() const {
	return Vector2(_width, _height);
}

HRESULT Deferredbuffers::createRenderTarget(uint16 index, DXGI_FORMAT format, ComPtr<ID3D11Device>& device) {
	
	HRESULT result;
	
	//レンダーターゲットを生成
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = _width;
	textureDesc.Height = _height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, NULL, _renderTargetTextureArray[index].ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//レンダーターゲットビューの生成
	D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
	ZeroMemory(&renderDesc, sizeof(renderDesc));
	renderDesc.Format = format;
	renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderDesc.Texture2D.MipSlice = 0;
	
	result = device->CreateRenderTargetView(_renderTargetTextureArray[index].Get(), &renderDesc, _renderTargetViewArray[index].ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = format;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(_renderTargetTextureArray[index].Get(), &shaderDesc, _shaderResourceViewArray[index].ReleaseAndGetAddressOf());
	if (FAILED(result)) {
		return result;
	}
	
	return result;
}
