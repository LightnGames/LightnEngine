#pragma once

#include <d3d11.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <Util/ComPtr.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/Mesh/Skeleton.h>

enum class MeshType{
	None,
	Static,
	Skeletal,
	Animation
};

class MeshLoader{

public:

	MeshLoader(const ComPtr<ID3D11Device>& device,const ComPtr<ID3D11DeviceContext>& deviceContext);

	~MeshLoader();

	//FBXファイル読み込み
	HRESULT load(const std::string& filePath, const std::vector<std::string>& matFiles);

	//スケルトンデータ読み込み
	std::unique_ptr<Skeleton> loadSkeleton();

	//メッシュデータを取得
	LocalMesh getMeshes() const;

	//メッシュタイプを取得
	MeshType getType() const;

private:

	//マテリアルデータからGPUバッファを生成
	HRESULT setupMaterial();

	//マテリアルデータをファイルから読み込み
	HRESULT loadMaterialFromFile(const std::vector<std::string>& fileName);

	//シェーダーと定数バッファを生成
	HRESULT createShaderAndConstantBuffer(std::shared_ptr<MaterialData>& material, bool isSkeletalMesh);

	//このメッシュにおけるフルパスを取得
	std::string getFullPath(const char* fileName) const;

	//スケルタルメッシュか？
	bool isSkeletalMesh();

	//ファイルからメッシュデータ読み込み
	void loadMaterialAndVertexData(std::ifstream& fin);

private:

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;

	std::string _folderPath;
	std::string _filePath;

	//メッシュデータ
	LocalMesh _mesh;

	MeshType _type;

	std::unique_ptr<Skeleton> _skeleton;

	int _materialSlotCount;
};