#pragma once

#include <d3d11.h>
#include <Util/Type.h>
#include <Util/ComPtr.h>

const int BUFFER_COUNT = 3;

class Deferredbuffers {

public:

	Deferredbuffers();
	~Deferredbuffers();

	//GBuffer初期化
	HRESULT initialize(ComPtr<ID3D11Device>& device, uint16 width, uint16 height, float screenDepth, float screenNear);

	//GBufferをすべて削除
	void cleanUp();

	//GBufferのカラーバッファをメインレンダーターゲットに設定
	void setRenderTargets(ComPtr<ID3D11DeviceContext> context);

	//ZPrePass用のレンダーターゲットをセット
	void setRenderTargetEaryZ(ComPtr<ID3D11DeviceContext> context);

	//GBufferを指定カラーでクリア
	void clearRenderTargets(ComPtr<ID3D11DeviceContext> context, float red, float green, float blue, float alpha);

	//Gbufferのシェーダーリソースビューをインデックスで取得
	ComPtr<ID3D11ShaderResourceView> getShaderResourceView(uint16 index) const;

	ComPtr<ID3D11ShaderResourceView> getDepthStencilResource() const;

private:

	//個々のレンダーターゲットをフォーマットとインデックスで生成
	HRESULT createRenderTarget(uint16 index, DXGI_FORMAT format, ComPtr<ID3D11Device>& device);

public:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Texture2D> _renderTargetTextureArray[BUFFER_COUNT];
	ComPtr<ID3D11RenderTargetView> _renderTargetViewArray[BUFFER_COUNT];
	ComPtr<ID3D11ShaderResourceView> _shaderResourceViewArray[BUFFER_COUNT];
	ComPtr<ID3D11Texture2D> _depthStencilBuffer;
	ComPtr<ID3D11ShaderResourceView> _depthStencilResource;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	ComPtr<ID3D11DepthStencilState> _depthStencilState;
	ComPtr<ID3D11PixelShader> _depthShader;
	D3D11_VIEWPORT _viewport;

};