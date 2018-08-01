#pragma once

#include <Renderer/RenderableObject.h>
#include <Util/ComPtr.h>
#include <Util/RefPtr.h>

#include <d3d11.h>
#include <LMath.h>
#include <Util/Type.h>
#include <vector>
#include <Collider/AABB.h>

#include "Mesh.h"

struct DrawSettings;
struct BoundingBoxInfo;
struct StaticInstanceMeshData;

class StaticInstanceMesh :public RenderableObject {

public:

	StaticInstanceMesh(const LocalMesh& meshes);

	void setUp(ComPtr<ID3D11Device> device, const std::vector<Matrix4>& matrices, uint32 meshDrawOffset, uint32 matrixBufferOffset);

	//メッシュを描画
	void draw(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData);

	void drawDepth(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData);

	RefPtr<MaterialData> material(int index) const;

	RefPtr<const LocalMesh> meshInfo() const;

protected:

	//メッシュデータ
	LocalMesh _meshes;

	ComPtr<ID3D11Buffer> _meshDrawOffsetConstantBuffer;
	ComPtr<ID3D11Buffer> _boundingBoxListBuffer;
	ComPtr<ID3D11ShaderResourceView> _boundingBoxListSRV;

	uint32 _instanceCount;
	uint32 _paddingedCount;
	uint32 _meshDrawOffset;
	uint32 _matrixBufferOffset;

	const uint32 THREAD_NUM = 4;

	std::vector<AABB> _cullingBoxes;

};