#pragma once

#include <d3d11.h>
#include <string>

#include <Util/Singleton.h>
#include <Util/RefPtr.h>
#include <Util/ComPtr.h>
#include <LMath.h>
#include <Util/Type.h>

class OrthoScreen {

public:
	OrthoScreen();
	~OrthoScreen();

	HRESULT initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext, ComPtr<IDXGISwapChain>& swapChain, uint16 width, uint16 height);

	//レンダーターゲットをバックバッファに切り替え
	void setBackBuffer();

	//バックバッファとステンシルビューをセット
	void setBackBufferAndDSV(ComPtr<ID3D11DepthStencilView> dsv);

	//ライティング用ステンシルステートをセット
	void setStencilStateLight();

	//メインレンダーターゲットをクリア
	void clearMainRenderTarget(const float color[4]);

	//クリーンアップ
	void cleanUp();

	//スクリーン描画用頂点バッファに切り替え
	void setOrthoScreenVertex();

	//メインレンダーターゲットのシェーダーリソースビューを取得
	ComPtr<ID3D11ShaderResourceView> getShaderResourceView() const;

private:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;

	ComPtr<ID3D11DepthStencilState> _stencilState;
	ComPtr<ID3D11Buffer> _screenVertexBuffer;
	ComPtr<ID3D11BlendState> _blendState;
	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
	D3D11_VIEWPORT _viewPort;
};