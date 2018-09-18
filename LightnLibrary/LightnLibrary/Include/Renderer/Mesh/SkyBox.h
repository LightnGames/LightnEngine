#pragma once

#include <Renderer/Mesh/SkeletalMesh.h>
#include <Util/Util.h>

#include <d3d11.h>
#include <LMath.h>
#include <vector>

#include <Renderer/Mesh/Mesh.h>

struct DrawSettings;

class SkyBox :public StaticMesh {

public:

	SkyBox(const LocalMesh& meshes, ComPtr<ID3D11Device> device);

	//ƒƒbƒVƒ…‚ğ•`‰æ
	virtual void drawStencil(const DrawSettings& drawSettings);

	virtual void draw(const DrawSettings& drawSettings);

	ComPtr<ID3D11ShaderResourceView> getSkyBoxCubemapResource() { return material(0)->ppTextures[0]; }

private:

	ComPtr<ID3D11DepthStencilState> _stencilWriteState;
	ComPtr<ID3D11DepthStencilState> _stencilReadState;
};