#pragma once

#include <d3d11.h>
#include <Util/ComPtr.h>
#include <Util/Type.h>
#include <LMath.h>
#include <Util/RefPtr.h>
#include <string>

struct DrawSettings;
class LightComponent;


struct ShadowResource {

	ComPtr<ID3D11Texture2D> _depthStencilBuffer;
	ComPtr<ID3D11ShaderResourceView> _depthStencilSRV;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	ComPtr<ID3D11SamplerState> _shadowSampler;
	D3D11_VIEWPORT shadowVp;

	ShadowResource(uint32 size, float scale = 50.0f, float nearZ = 0.1f, float farZ = 500.0f) :
		shadowMapWidth{ size },
		shadowMapHeight{ size },
		scale{ scale },
		nearZ{ nearZ },
		farZ {farZ} {}

	HRESULT initialize(ComPtr<ID3D11Device> device);

	uint32 shadowMapWidth;
	uint32 shadowMapHeight;

	float scale;
	float nearZ;
	float farZ;
};

class Light {

public:

	Light();

	virtual ~Light();

	virtual void draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent);

	ShadowResource createShadow(uint32 size);

protected:

	void initializeLight(ComPtr<ID3D11Device>& device, const std::string& vertexShader, const std::string& pixelShader);

protected:

	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11InputLayout> _layout;
	ComPtr<ID3D11SamplerState> _sampleState;
	ComPtr<ID3D11Buffer> _matrixBuffer;
	ComPtr<ID3D11Buffer> _lightBuffer;

};