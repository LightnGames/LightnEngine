#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>
#include <vector>

#include <Renderer/Mesh/Mesh.h>

#define MAX_INSTANCE_NUM 512

struct DebugGeometoryInfo {
	Vector4 color;
	Matrix4 mtxWorld;
};

struct DrawSettings;

using DebugDrawList = std::vector<DebugGeometoryInfo>;

class DebugGeometry {

public:

	void initialize(const void* vertices, uint32 vSize, const void* indices, uint32 iSize, ComPtr<ID3D11Device>& device);

	void draw(const DrawSettings & drawSettings, const DebugGeometoryInfo* info, uint32 instanceCount);

	//バッファデータ
	ComPtr<ID3D11Buffer>             _vertexBuffer;
	ComPtr<ID3D11Buffer>             _debugGeometryInfoBuffer;
	ComPtr<ID3D11Buffer>             _indexBuffer;
	uint32 _indexCount;
};

class DebugGeomtryRenderer {

public:

	void initialize(ComPtr<ID3D11Device>& device);

	void draw(const DebugDrawList& sphere, const DebugDrawList& box, const DebugDrawList& line, const DrawSettings & drawSettings);

	std::unique_ptr<DebugGeometry> _sphereMesh;
	std::unique_ptr<DebugGeometry> _boxMesh;
	std::unique_ptr<DebugGeometry> _lineMesh;

	ComPtr<ID3D11VertexShader>       _vertexShader;
	ComPtr<ID3D11InputLayout>        _vertexLayout;
	ComPtr<ID3D11PixelShader>        _pixelShader;
	ComPtr<ID3D11Buffer>             _constantBuffer;
	ComPtr<ID3D11RasterizerState>    _rasterizerState;
};