#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>
#include <memory>

class RenderTarget;
const int BUFFER_COUNT = 4;

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

	//ライティング結果用のレンダーターゲットに切り替え
	void setRenderTargetLighting(ComPtr<ID3D11DeviceContext> deviceContext);

	//ZPrePass用のレンダーターゲットをセット
	void setRenderTargetEaryZ(ComPtr<ID3D11DeviceContext> context);

	//GBufferを指定カラーでクリア
	void clearRenderTargets(ComPtr<ID3D11DeviceContext> context, float red, float green, float blue, float alpha);

	void setViewPort(ComPtr<ID3D11DeviceContext> deviceContext);

	ID3D11ShaderResourceView* getShaderResourceView(uint16 index) const;

	RefPtr<RenderTarget> getRenderTarget(uint16 index);

	ComPtr<ID3D11ShaderResourceView> getDepthStencilResource() const;

	ID3D11DepthStencilView* getDepthStencilView(bool readOnly);

	Vector2 getGBufferSize() const;

private:

	//個々のレンダーターゲットをフォーマットとインデックスで生成
	HRESULT createRenderTarget(uint16 index, DXGI_FORMAT format, ComPtr<ID3D11Device>& device);

private:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Texture2D> _depthStencilBuffer;
	ComPtr<ID3D11ShaderResourceView> _depthStencilSRV;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	ComPtr<ID3D11DepthStencilView> _depthStencilViewReadOnly;
	ComPtr<ID3D11DepthStencilState> _depthStencilState;
	std::unique_ptr<RenderTarget> _renderTargets[BUFFER_COUNT];
	D3D11_VIEWPORT _viewport;

};