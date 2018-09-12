#include <Renderer/GraphicsBuffers.h>
#include <cassert>

RenderTarget::RenderTarget(ID3D11Device* device, DXGI_FORMAT format, UINT width, UINT height, UINT flags) :
	_texture{ nullptr },
	_rtv{ nullptr },
	_srv{ nullptr } {

	HRESULT hr;

	//レンダーターゲットを生成
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = flags;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = device->CreateTexture2D(&textureDesc, 0, &_texture);
	assert(SUCCEEDED(hr) && "レンダーターゲットのテクスチャ生成に失敗");

	if (flags & D3D11_BIND_RENDER_TARGET) {

		//レンダーターゲットビューの生成
		D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
		ZeroMemory(&renderDesc, sizeof(renderDesc));
		renderDesc.Format = format;
		renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderDesc.Texture2D.MipSlice = 0;

		hr = device->CreateRenderTargetView(_texture, &renderDesc, &_rtv);
		assert(SUCCEEDED(hr) && "レンダーターゲットのRTV生成に失敗");
	}

	if (flags & D3D11_BIND_SHADER_RESOURCE) {

		//シェーダーリソースビューの生成
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		ZeroMemory(&shaderDesc, sizeof(shaderDesc));
		shaderDesc.Format = format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(_texture, &shaderDesc, &_srv);
		assert(SUCCEEDED(hr) && "レンダーターゲットのSRV生成に失敗");
	}

}

RenderTarget::~RenderTarget() {

	_texture->Release();
	_texture = nullptr;

	if (_rtv != nullptr) {
		_rtv->Release();
		_rtv = nullptr;
	}

	if (_srv != nullptr) {
		_srv->Release();
		_srv = nullptr;
	}
}