#pragma once

#include <Renderer/RenderableObject.h>
#include <Util/ComPtr.h>
#include <Util/RefPtr.h>

#include <d3d11.h>
#include <LMath.h>
#include <vector>

#include "Mesh.h"

struct DrawSettings;

class StaticMesh :public RenderableObject {

public:

	StaticMesh(const LocalMesh& meshes);

	void setUp(ComPtr<ID3D11Device> device);

	//メッシュを描画
	virtual void draw(const DrawSettings& drawSettings, const Matrix4& worldMatrix);

	virtual void drawDepth(const DrawSettings& drawSettings, const Matrix4& worldMatrix);

	RefPtr<MaterialData> material(int index);

protected:

	//メッシュを描画
	void drawMesh(ComPtr<ID3D11DeviceContext> deviceContext, const RefPtr<void>& constantBuffer, const UINT vertexBufferSize);

protected:

	//メッシュデータ
	LocalMesh _meshes;

};