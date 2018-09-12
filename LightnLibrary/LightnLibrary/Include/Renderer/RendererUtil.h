#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/Camera.h>

class BinaryLoader;

class RendererUtil{

public:

	//各種シェーダー作成
	static HRESULT createVertexShader(const LPCSTR& fileName, ComPtr<ID3D11VertexShader>& ppVertexShader, const D3D11_INPUT_ELEMENT_DESC layout[], const uint32 numElements, ComPtr<ID3D11InputLayout>& ppInputLayout, ComPtr<ID3D11Device>& pDevice);
	static HRESULT createPixelShader(const LPCSTR& fileName, ComPtr<ID3D11PixelShader>& ppPixelShader, ComPtr<ID3D11Device>& pDevice);
	static HRESULT createComputeShader(const LPCSTR& fileName, ComPtr<ID3D11ComputeShader>& computeShader, ComPtr<ID3D11Device>& device);

	//コンスタントバッファ作成
	static HRESULT createConstantBuffer(ComPtr<ID3D11Buffer>& ppConstantBuffer, const uint32 size, ComPtr<ID3D11Device>& pDevice, const void* initPtr = nullptr);

	//頂点バッファ生成
	static HRESULT createVertexBuffer(const void * vertices, uint32 size, ComPtr<ID3D11Buffer>& vertexBuffer, ComPtr<ID3D11Device>& pDevice, D3D11_CPU_ACCESS_FLAG cpuFlag = (D3D11_CPU_ACCESS_FLAG)0);

	//インデックスバッファ生成
	static HRESULT createIndexBuffer(const void * indices, uint32 indexCount, ComPtr<ID3D11Buffer>& indexBuffer, ComPtr<ID3D11Device>& pDevice);

	//頂点バッファ用定数バッファを取得
	static MeshConstantBuffer getConstantBuffer(const Matrix4 & mtxWorld, RefPtr<Camera> camera);

	//シェーダーパスを取得
	static BinaryLoader getShaderPath(const std::string& shaderName);
};