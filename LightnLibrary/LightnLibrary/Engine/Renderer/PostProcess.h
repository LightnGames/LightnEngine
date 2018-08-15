#pragma once

#include <d3d11.h>
#include <Util/ComPtr.h>
#include <Util/RefPtr.h>
#include <Util/Type.h>
#include <LMath.h>

#define BLOOM_DOWN_SAMPLE 4

class Deferredbuffers;
class OrthoScreen;

struct GaussBlurParam {
	uint32 sampleCount;
	uint32 sampleCountDummy1;
	uint32 sampleCountDummy2;
	uint32 sampleCountDummy3;
	Vector4 offset[16];
};

class PostProcess {

public:

	void initialize(ComPtr<ID3D11Device> device);

	HRESULT setupRenderResource(ComPtr<ID3D11Device> device, uint16 width, uint16 height);

	void draw(ComPtr<ID3D11DeviceContext> deviceContext, RefPtr<Deferredbuffers> deferredBuffers, RefPtr<OrthoScreen> orthoScreen);

private:

	inline GaussBlurParam CalcBlurParam(uint32 width, uint32 height, Vector2 dir, float deviation, float multiply);

	inline float GaussianDistribution(const Vector2& pos, float rho);

	void createRenderTargets(ComPtr<ID3D11Texture2D>& tex, ComPtr<ID3D11RenderTargetView>& rtv, ComPtr<ID3D11ShaderResourceView>& srv, uint32 width, uint32 height, ComPtr<ID3D11Device> device);

private:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11PixelShader> postProcessShader;
	ComPtr<ID3D11PixelShader> gaussianBlurShader;
	ComPtr<ID3D11Buffer> gaussianConstantBuffer;
	ComPtr<ID3D11BlendState> _blendState;
	ComPtr<ID3D11SamplerState> _linerSampler;
	ComPtr<ID3D11SamplerState> _pointSampler;
	ComPtr<ID3D11RenderTargetView> bloomDownSampleRTV[BLOOM_DOWN_SAMPLE * 2];
	ComPtr<ID3D11Texture2D> bloomDownSampleTex[BLOOM_DOWN_SAMPLE * 2];
	ComPtr<ID3D11ShaderResourceView> bloomDownSampleSRV[BLOOM_DOWN_SAMPLE * 2];
};