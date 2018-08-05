#include "RendererUtil.h"
#include "../Engine/Loader/BinaryLoader.h"
#include <string>
#include <Util/RefPtr.h>
#include <Components/CameraComponent.h>


HRESULT RendererUtil::createVertexShader(const LPCSTR & fileName, ComPtr<ID3D11VertexShader>& ppVertexShader, const D3D11_INPUT_ELEMENT_DESC layout[], const uint32 numElements, ComPtr<ID3D11InputLayout>& ppInputLayout, ComPtr<ID3D11Device>& pDevice) {

	//���_�V�F�[�_�[�t�@�C���ǂݍ���
	BinaryLoader vsB = getShaderPath(fileName);

	HRESULT hr = pDevice->CreateVertexShader(vsB.data(), vsB.size(), NULL, ppVertexShader.GetAddressOf());

	if (FAILED(hr)) {
		throw std::runtime_error("failed create vertex shader ");
	}

	//UINT numElements = ARRAYSIZE (layout);
	auto temp = pDevice->CreateInputLayout(layout, numElements, vsB.data(), vsB.size(), ppInputLayout.GetAddressOf());

	return hr;
}

HRESULT RendererUtil::createPixelShader(const LPCSTR & fileName, ComPtr<ID3D11PixelShader>& ppPixelShader, ComPtr<ID3D11Device>& pDevice) {
	//�s�N�Z���V�F�[�_�[�ǂݍ���
	BinaryLoader psB = getShaderPath(fileName);

	//�s�N�Z���V�F�[�_�[�쐬
	HRESULT hr = pDevice->CreatePixelShader(psB.data(), psB.size(), NULL, ppPixelShader.GetAddressOf());

	if (FAILED(hr)) {
		throw std::runtime_error("failed create pixel shader ");
	}

	return hr;
}

HRESULT RendererUtil::createComputeShader(const LPCSTR & fileName, ComPtr<ID3D11ComputeShader>& computeShader, ComPtr<ID3D11Device>& device)
{
	BinaryLoader psB = getShaderPath(fileName);
	HRESULT hr = device->CreateComputeShader(psB.data(), psB.size(), NULL, computeShader.GetAddressOf());

	return hr;
}

HRESULT RendererUtil::createConstantBuffer(ComPtr<ID3D11Buffer>& constantBuffer, const uint32 size, ComPtr<ID3D11Device>& device, const void* initPtr) {
	
	//�R���X�^���g�o�b�t�@�\�쐬
	D3D11_BUFFER_DESC cb;
	ZeroMemory(&cb, sizeof(cb));
	cb.Usage = D3D11_USAGE_DEFAULT;
	cb.ByteWidth = size;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = 0;

	HRESULT hr;
	if (initPtr == nullptr) {
		
		hr = device->CreateBuffer(&cb, nullptr, constantBuffer.GetAddressOf());
		
	} else {

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initPtr;
		hr = device->CreateBuffer(&cb, &initData, constantBuffer.GetAddressOf());
	}

	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

HRESULT RendererUtil::createVertexBuffer(const void * vertices, uint32 size, ComPtr<ID3D11Buffer>& vertexBuffer, ComPtr<ID3D11Device>& pDevice, D3D11_CPU_ACCESS_FLAG cpuFlag) {

	//���_�o�b�t�@�쐬
	D3D11_BUFFER_DESC bdV;
	ZeroMemory(&bdV, sizeof(bdV));
	bdV.Usage = D3D11_USAGE_DEFAULT;
	bdV.ByteWidth = size;
	bdV.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bdV.CPUAccessFlags = cpuFlag;
	bdV.MiscFlags = 0;

	//CPU�A�N�Z�X�`�F�b�N
	switch (cpuFlag)
	{
	case 0:
		bdV.Usage = D3D11_USAGE_DEFAULT;
		bdV.CPUAccessFlags = 0;
		break;
	default:
		bdV.Usage = D3D11_USAGE_DYNAMIC;
		bdV.CPUAccessFlags = cpuFlag;
		break;
	}

	bdV.Usage = D3D11_USAGE_DEFAULT;
	bdV.CPUAccessFlags = 0;

	HRESULT hr;
	if (vertices != nullptr) {
		D3D11_SUBRESOURCE_DATA initDataV;
		initDataV.pSysMem = vertices;
		hr = pDevice->CreateBuffer(&bdV, &initDataV, vertexBuffer.ReleaseAndGetAddressOf());
	}
	else {
		hr = pDevice->CreateBuffer(&bdV, NULL, vertexBuffer.ReleaseAndGetAddressOf());
	}

	if (FAILED(hr)) {
		//throw std::runtime_error("failed create vertex buffer ");
		return hr;
	}

	return S_OK;
}

HRESULT RendererUtil::createIndexBuffer(const void * indices, uint32 indexCount, ComPtr<ID3D11Buffer>& indexBuffer, ComPtr<ID3D11Device>& pDevice) {

	//�}�e���A�����Ƃ̃C���f�b�N�X�o�b�t�@�쐬
	D3D11_BUFFER_DESC bdI;
	ZeroMemory(&bdI, sizeof(bdI));
	bdI.ByteWidth = sizeof(int)*indexCount;
	bdI.Usage = D3D11_USAGE_DEFAULT;
	bdI.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bdI.CPUAccessFlags = 0;
	bdI.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initDataI;
	initDataI.pSysMem = indices;
	HRESULT hr = pDevice->CreateBuffer(&bdI, &initDataI, indexBuffer.ReleaseAndGetAddressOf());

	if (FAILED(hr)) {
		//throw std::runtime_error("failed create index buffer ");
		return hr;
	}

	return S_OK;
}

MeshConstantBuffer RendererUtil::getConstantBuffer(const Matrix4 & mtxWorld)
{
	const RefPtr<CameraComponent>& camera = CameraComponent::mainCamera;

	MeshConstantBuffer constantBuffer;

	//�ˉe�ϊ��s����Z�b�g
	constantBuffer.mtxProj = camera->mtxProj().transpose();

	//�J�����r���[�s����Z�b�g
	constantBuffer.mtxView = Matrix4::transpose(camera->cameraMatrix().inverse());

	//world�s����Z�b�g
	constantBuffer.mtxWorld = Matrix4::transpose(mtxWorld);

	//�J�������W���Z�b�g
	constantBuffer.cameraPos = Vector4(camera->getWorldPosition());

	return std::move(constantBuffer);
}

BinaryLoader RendererUtil::getShaderPath(const std::string & shaderName)
{
	std::string shaderPath("Shader/");
	shaderPath += shaderName;

	return BinaryLoader(shaderPath.c_str());
}
