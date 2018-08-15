#include "OrthoScreen.h"
#include <Renderer/RendererUtil.h>

OrthoScreen::OrthoScreen():
	_width{0},
	_height{0},
	_device{nullptr},
	_deviceContext{nullptr},
	_screenVertexBuffer{nullptr},
	_renderTargetView{nullptr},
	_shaderResourceView{nullptr},
	_viewPort{0} {
}

OrthoScreen::~OrthoScreen() {
}

HRESULT OrthoScreen::initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext, ComPtr<IDXGISwapChain>& swapChain, uint16 width, uint16 height) {
	
	HRESULT hr;

	_width = width;
	_height = height;

	_device = device;
	_deviceContext = deviceContext;

	//バックバッファの生成
	ComPtr<ID3D11Texture2D> pBack;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)pBack.ReleaseAndGetAddressOf());

	//レンダーターゲットビューの作成
	device->CreateRenderTargetView(pBack.Get(), NULL, _renderTargetView.ReleaseAndGetAddressOf());

	//ビューポートの作成
	_viewPort.Width = static_cast<FLOAT>(_width);
	_viewPort.Height = static_cast<FLOAT>(_height);
	_viewPort.MinDepth = 0.0f;
	_viewPort.MaxDepth = 1.0f;
	_viewPort.TopLeftX = 0;
	_viewPort.TopLeftY = 0;

	deviceContext->RSSetViewports(1, &_viewPort);

	//ライト加算用アルファブレンドステートを生成
	D3D11_BLEND_DESC BlendDesc;
	ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	_device->CreateBlendState(&BlendDesc, _blendState.GetAddressOf());

	//シェーダーリソースビューの生成
	hr = device->CreateShaderResourceView(pBack.Get(), NULL, _shaderResourceView.ReleaseAndGetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

	// Create depth stencil state
	hr = device->CreateDepthStencilState(&dsDesc, _stencilState.ReleaseAndGetAddressOf());

	//ウィンドウ用の頂点バッファを生成
	const float screenBuffer[20] = {
		-1,-1, 0, 0, 1,
		-1, 1, 0, 0, 0,
		1,-1, 0, 1, 1,
		1, 1, 0, 1, 0
	};

	const uint32 size = sizeof(float) * 5;
	RendererUtil::createVertexBuffer(&screenBuffer, size * 4, _screenVertexBuffer, _device);

	return S_OK;
}

void OrthoScreen::setBackBuffer() {
	
	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), 0);
	
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_deviceContext->OMSetBlendState(_blendState.Get(), blendFactor, 0xffffffff);
	_deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void OrthoScreen::setBackBufferAndDSV(ComPtr<ID3D11DepthStencilView> dsv) {
	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), dsv.Get());

	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_deviceContext->OMSetBlendState(_blendState.Get(), blendFactor, 0xffffffff);
}

void OrthoScreen::setStencilStateLight() {
	_deviceContext->OMSetDepthStencilState(_stencilState.Get(), 1);
}

void OrthoScreen::clearMainRenderTarget(const float color[4]) {
	_deviceContext->ClearRenderTargetView(_renderTargetView.Get(), color);
}

void OrthoScreen::cleanUp() {

	if (_renderTargetView._pInterface != nullptr) {
		_renderTargetView->Release();
		_renderTargetView._pInterface = nullptr;
	}
	if (_shaderResourceView._pInterface != nullptr) {
		_shaderResourceView->Release();
		_shaderResourceView._pInterface = nullptr;
	}
}

void OrthoScreen::setOrthoScreenVertex() {

	const uint32 size = sizeof(float) * 5;
	const uint32 offset = 0;
	_deviceContext->IASetVertexBuffers(0, 1, _screenVertexBuffer.GetAddressOf(), &size, &offset);
}

ComPtr<ID3D11ShaderResourceView> OrthoScreen::getShaderResourceView() const {
	return _shaderResourceView;
}
