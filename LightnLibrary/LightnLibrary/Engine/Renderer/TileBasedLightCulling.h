#pragma once

#include <d3d11.h>
#include <Util/ComPtr.h>
#include <Util/RefPtr.h>
#include <Util/Type.h>
#include <LMath.h>

struct DrawSettings;

class TileBasedLightCulling {
public:

	TileBasedLightCulling();

	~TileBasedLightCulling();

	HRESULT initialize(ComPtr<ID3D11Device> device, uint32 width, uint32 height);

	void draw(const DrawSettings& settings);

private:

	ComPtr<ID3D11Buffer> _perFrameConstantBuffer;
	ComPtr<ID3D11Buffer> _pointLightListBuffer;
	ComPtr<ID3D11Buffer> _frameBuffer;
	ComPtr<ID3D11Buffer> _perTilePlaneBuffer;
	ComPtr<ID3D11ShaderResourceView> _perTilePlaneSRV;
	ComPtr<ID3D11ShaderResourceView> _pointLightListSRV;
	ComPtr<ID3D11ShaderResourceView> _frameBufferSRV;
	ComPtr<ID3D11UnorderedAccessView> _frameBufferUAV;
	ComPtr<ID3D11ComputeShader> _tileBasedLightCullingShader;
	ComPtr<ID3D11PixelShader> _structuredToRenderShader;
};