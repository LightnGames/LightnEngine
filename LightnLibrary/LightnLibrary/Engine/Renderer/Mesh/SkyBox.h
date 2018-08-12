#pragma once

#include <Renderer/Mesh/SkeletalMesh.h>
#include <Util/ComPtr.h>
#include <Util/RefPtr.h>

#include <d3d11.h>
#include <LMath.h>
#include <vector>

#include "Mesh.h"

struct DrawSettings;

class SkyBox :public StaticMesh {

public:

	SkyBox(const LocalMesh& meshes, ComPtr<ID3D11Device> device);

	//ƒƒbƒVƒ…‚ğ•`‰æ
	virtual void drawStencil(const DrawSettings& drawSettings);

	virtual void draw(const DrawSettings& drawSettings);

private:

	ComPtr<ID3D11DepthStencilState> _stencilWriteState;
	ComPtr<ID3D11DepthStencilState> _stencilReadState;
};