#pragma once

#include <d3d11.h>
#include <Util/ComPtr.h>
#include <Util/Type.h>
#include <LMath.h>
#include <Util/RefPtr.h>
#include <string>

struct DrawSettings;
class LightComponent;

class Light {

public:

	Light();

	virtual ~Light();

	virtual void draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent);
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