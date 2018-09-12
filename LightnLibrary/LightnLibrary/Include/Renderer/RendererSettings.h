#pragma once

#include <Util/Util.h>
#include <d3d11.h>

class RendererSettings {

public:
	RendererSettings();

	static ComPtr<ID3D11ShaderResourceView> skyBox;

};