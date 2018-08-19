#pragma once

#include <d3d11.h>

class RenderTarget {

public:

	RenderTarget(ID3D11Device* device, DXGI_FORMAT format, UINT width, UINT height, UINT flags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

	~RenderTarget();

	ID3D11Texture2D* texture() { return _texture; }
	ID3D11RenderTargetView* rtv() { return _rtv; }
	ID3D11ShaderResourceView* srv() { return _srv; }

	ID3D11Texture2D** ppTexture() { return &_texture; }
	ID3D11RenderTargetView** ppRtv() { return &_rtv; }
	ID3D11ShaderResourceView** ppSrv() { return &_srv; }

private:

	//ÉRÉsÅ[ã÷é~
	RenderTarget(const RenderTarget&);
	RenderTarget& operator = (const RenderTarget&);

	ID3D11Texture2D* _texture;
	ID3D11RenderTargetView* _rtv;
	ID3D11ShaderResourceView* _srv;
};