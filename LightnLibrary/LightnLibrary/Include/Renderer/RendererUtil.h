#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/Camera.h>

class BinaryLoader;

class RendererUtil{

public:

	//�e��V�F�[�_�[�쐬
	static HRESULT createVertexShader(const LPCSTR& fileName, ComPtr<ID3D11VertexShader>& ppVertexShader, const D3D11_INPUT_ELEMENT_DESC layout[], const uint32 numElements, ComPtr<ID3D11InputLayout>& ppInputLayout, ComPtr<ID3D11Device>& pDevice);
	static HRESULT createPixelShader(const LPCSTR& fileName, ComPtr<ID3D11PixelShader>& ppPixelShader, ComPtr<ID3D11Device>& pDevice);
	static HRESULT createComputeShader(const LPCSTR& fileName, ComPtr<ID3D11ComputeShader>& computeShader, ComPtr<ID3D11Device>& device);

	//�R���X�^���g�o�b�t�@�쐬
	static HRESULT createConstantBuffer(ComPtr<ID3D11Buffer>& ppConstantBuffer, const uint32 size, ComPtr<ID3D11Device>& pDevice, const void* initPtr = nullptr);

	//���_�o�b�t�@����
	static HRESULT createVertexBuffer(const void * vertices, uint32 size, ComPtr<ID3D11Buffer>& vertexBuffer, ComPtr<ID3D11Device>& pDevice, D3D11_CPU_ACCESS_FLAG cpuFlag = (D3D11_CPU_ACCESS_FLAG)0);

	//�C���f�b�N�X�o�b�t�@����
	static HRESULT createIndexBuffer(const void * indices, uint32 indexCount, ComPtr<ID3D11Buffer>& indexBuffer, ComPtr<ID3D11Device>& pDevice);

	//���_�o�b�t�@�p�萔�o�b�t�@���擾
	static MeshConstantBuffer getConstantBuffer(const Matrix4 & mtxWorld, RefPtr<Camera> camera);

	//�V�F�[�_�[�p�X���擾
	static BinaryLoader getShaderPath(const std::string& shaderName);
};