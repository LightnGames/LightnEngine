#pragma once

#include <d3d11.h>

#include <memory>
#include <vector>
#include <Util/ComPtr.h>
#include <LMath.h>
#include <Collider/AABB.h>

#define MAX_BONES 255

class CameraComponent;

//���_�\����
struct MeshVertex
{
	Vector3 pos;
	Vector2 tex;

	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;

	//���b�V���f�[�^���s��ŉ�]����
	void MeshTransform(const Matrix4& matrix){

		Matrix4 rotator = matrix;
		rotator.m[3][0] = 0.0f;
		rotator.m[3][1] = 0.0f;
		rotator.m[3][2] = 0.0f;

		pos = Matrix4::transform(pos, matrix);
		normal = Matrix4::transform(normal, rotator);
		binormal = Matrix4::transform(binormal, rotator);
		tangent = Matrix4::transform(tangent, rotator);
	}

	//���b�V���f�[�^���s��ňړ��E��]����
	void MeshTranslateAndScale(const Matrix4& matrix){
		pos = Matrix4::transform(pos, matrix);
	}
};

//�}�e���A���f�[�^�\����
struct MaterialData 
{
	//�p�����[�^�f�[�^
	std::string               vertexShaderFileName;
	std::string               pixelShaderFileName;
	std::vector<std::string>  textureFileNames;
	UINT                      textureCount;
	UINT                      alphaType;
	D3D11_CULL_MODE           cullMode;

	//�o�b�t�@�f�[�^
	ComPtr<ID3D11VertexShader>       pVertexShader;
	ComPtr<ID3D11InputLayout>        pVertexLayout;
	ComPtr<ID3D11PixelShader>        pPixelShader;
	ComPtr<ID3D11Buffer>             pConstantBuffer;
	ComPtr<ID3D11Buffer>             pIndexBuffer;

	//�e�N�X�`���f�[�^
	std::vector<ComPtr<ID3D11ShaderResourceView>> ppTextures;
	ComPtr<ID3D11SamplerState>					  pSamplerLiner;

	//���̃}�e���A���̃|���S����
	UINT faceCount;
};

struct LocalMesh{

	//���_�o�b�t�@
	ComPtr<ID3D11Buffer> vertexBuffer;

	//�}�e���A���f�[�^�z��
	std::vector<std::shared_ptr<MaterialData>> materialSlots;

	//�o�E���f�B���O�{�b�N�X
	AABB boundingBox;
};

//���_�V�F�[�_�[�R���X�^���g�o�b�t�@�̍\����
struct MeshConstantBuffer
{
	Matrix4 mtxProj;
	Matrix4 mtxView;
	Matrix4 mtxWorld;
	Vector4 cameraPos;
};

struct BoundingBoxInfo
{
	BoundingBoxInfo(const Vector4& extent, const Matrix4& mtxWorld) :
		extent{ extent }, mtxWorld{ mtxWorld } {
	}

	Vector4 extent;
	Matrix4 mtxWorld;
};

struct Plane {
	Vector4 normal;
	Vector4 position;
};

struct FrustumCullingInfo {
	Plane planes[6];
};