#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>

class RenderTarget;
class Deferredbuffers;
class OrthoScreen;
struct Camera;

class SSAO {
public:

	void initialize(ComPtr<ID3D11Device> device);

	//サイズを指定してテクスチャを作成
	HRESULT setupRenderResource(ComPtr<ID3D11Device> device, uint16 width, uint16 height);

	void draw(ComPtr<ID3D11DeviceContext> deviceContext, RefPtr<Deferredbuffers> deferredBuffers, RefPtr<OrthoScreen> orthoScreen, RefPtr<Camera> camera);

	ID3D11ShaderResourceView** ssaoResource();

private:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Buffer> _postProcessConstantPP;
	ComPtr<ID3D11Buffer> _gaussianConstant;
	std::unique_ptr<RenderTarget> _ssaoRt;
	std::unique_ptr<RenderTarget> _ssaoRt2;
	ComPtr<ID3D11PixelShader> _ssaoShader;
	ComPtr<ID3D11PixelShader> _bilateralFilterShader;
	ComPtr<ID3D11SamplerState> _linerSampler;
};