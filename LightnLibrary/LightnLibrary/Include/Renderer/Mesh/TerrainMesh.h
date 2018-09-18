#pragma once
#include "StaticInstanceMesh.h"

class TerrainMesh :public StaticInstanceMesh {

public:

	TerrainMesh(const LocalMesh& meshes) :StaticInstanceMesh(meshes) {}

	void setUp(ComPtr<ID3D11Device> device, const std::vector<Matrix4>& matrices, uint32 meshDrawOffset, uint32 matrixBufferOffset) override;

	virtual void draw(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData) override;
	virtual void drawDepth(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData) override;

	ComPtr<ID3D11Texture2D> _heightTexture;
	ComPtr<ID3D11ShaderResourceView> _heightSRV;
};