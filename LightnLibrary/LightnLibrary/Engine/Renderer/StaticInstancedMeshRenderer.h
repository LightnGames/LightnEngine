#pragma once

#include <d3d11.h>
#include <Util/ComPtr.h>
#include <Util/Type.h>
#include <Util/RefPtr.h>
#include <Util/Singleton.h>
#include <Renderer/Mesh/Mesh.h>

struct Camera;

struct StaticInstanceMeshData {
	ComPtr<ID3D11ShaderResourceView> instanceMatrixSRV;
	ComPtr<ID3D11Buffer> indtsnceDrawListBuffer;
	ComPtr<ID3D11Buffer> instanceMatrixBuffer;
	ComPtr<ID3D11Buffer> frustumCullingBuffer;
	ComPtr<ID3D11UnorderedAccessView> instanceMatrixUAV;
	ComPtr<ID3D11UnorderedAccessView> instanceDrawListUAV;
	ComPtr<ID3D11ComputeShader> cullingShader;
	ComPtr<ID3D11ComputeShader> clearInstanceListShader;
};

class StaticInstancedMeshRenderer :public Singleton<StaticInstancedMeshRenderer> {

public:

	StaticInstancedMeshRenderer();

	HRESULT initialize(ComPtr<ID3D11Device> device, uint32 maxDrawCount, const std::vector<uint32>& indexList);

	void clearCullingBuffer(ComPtr<ID3D11DeviceContext> deviceContext, const Camera& camera);

	void clearCullingBufferShadow(ComPtr<ID3D11DeviceContext> deviceContext, const Camera& camera);

	RefPtr<StaticInstanceMeshData> getInstanceBuffers() const;

private:

	void calculateFrustumPlanes(Plane planes[6], const Camera& camera);

private:
	
	std::unique_ptr<StaticInstanceMeshData> _data;
	uint32 _meshTypeCount;
};