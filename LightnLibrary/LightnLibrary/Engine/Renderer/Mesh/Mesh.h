#pragma once

#include <d3d11.h>

#include <memory>
#include <vector>
#include <Util/ComPtr.h>
#include <LMath.h>
#include <Collider/AABB.h>

#define MAX_BONES 255

class CameraComponent;

//頂点構造体
struct MeshVertex
{
	Vector3 pos;
	Vector2 tex;

	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;

	//メッシュデータを行列で回転する
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

	//メッシュデータを行列で移動・回転する
	void MeshTranslateAndScale(const Matrix4& matrix){
		pos = Matrix4::transform(pos, matrix);
	}
};

//マテリアルデータ構造体
struct MaterialData 
{
	//パラメータデータ
	std::string               vertexShaderFileName;
	std::string               pixelShaderFileName;
	std::vector<std::string>  textureFileNames;
	UINT                      textureCount;
	UINT                      alphaType;
	D3D11_CULL_MODE           cullMode;

	//バッファデータ
	ComPtr<ID3D11VertexShader>       pVertexShader;
	ComPtr<ID3D11InputLayout>        pVertexLayout;
	ComPtr<ID3D11PixelShader>        pPixelShader;
	ComPtr<ID3D11Buffer>             pConstantBuffer;
	ComPtr<ID3D11Buffer>             pIndexBuffer;

	//テクスチャデータ
	std::vector<ComPtr<ID3D11ShaderResourceView>> ppTextures;
	ComPtr<ID3D11SamplerState>					  pSamplerLiner;

	//このマテリアルのポリゴン数
	UINT faceCount;
};

struct LocalMesh{

	//頂点バッファ
	ComPtr<ID3D11Buffer> vertexBuffer;

	//マテリアルデータ配列
	std::vector<std::shared_ptr<MaterialData>> materialSlots;

	//バウンディングボックス
	AABB boundingBox;
};

//頂点シェーダーコンスタントバッファの構造体
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