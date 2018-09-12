#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>

#define BLOOM_DOWN_SAMPLE 4

class RenderTarget;
class Deferredbuffers;
class OrthoScreen;
struct Camera;

struct GaussBlurParam {
	uint32 sampleCount;
	uint32 sampleCountDummy1;
	uint32 sampleCountDummy2;
	uint32 sampleCountDummy3;
	Vector4 offset[16];

	GaussBlurParam(uint32 width, uint32 height, Vector2 dir, float range = 35.0f);
};

class PostProcess {

public:

	void initialize(ComPtr<ID3D11Device> device);

	//サイズを指定してテクスチャを作成
	HRESULT setupRenderResource(ComPtr<ID3D11Device> device, uint16 width, uint16 height);

	void draw(ComPtr<ID3D11DeviceContext> deviceContext, RefPtr<Deferredbuffers> deferredBuffers, RefPtr<OrthoScreen> orthoScreen, RefPtr<Camera> camera);

public:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11PixelShader> _postProcessShader;
	ComPtr<ID3D11PixelShader> _gaussianBlurShader;
	ComPtr<ID3D11Buffer> _gaussianConstantBuffer;
	ComPtr<ID3D11BlendState> _blendState;
	ComPtr<ID3D11SamplerState> _linerSampler;
	std::unique_ptr<RenderTarget> _bloomDownSamples[BLOOM_DOWN_SAMPLE * 2];
};