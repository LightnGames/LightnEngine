#include "GraphicsResourceManager.h"
#include "RenderableObject.h"
#include <Loader/MeshLoader.h>
#include "GameRenderer.h"
#include "Mesh/StaticMesh.h"
#include "Mesh/SkeletalMesh.h"
#include "Mesh/StaticInstanceMesh.h"
#include <DirectXTex.h>
#include <functional>
#include "Light/DirectionalLight.h"
#include "Light/SkyLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"

template<> GraphicsResourceManager* Singleton<GraphicsResourceManager>::mSingleton = 0;

GraphicsResourceManager::GraphicsResourceManager() {
}

GraphicsResourceManager::~GraphicsResourceManager() {
}

void GraphicsResourceManager::initialize(ComPtr<ID3D11Device> device) {
	
	_device = device;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MaxAnisotropy = 16;//ミップマップの遠近レベル
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_device->CreateSamplerState(&sampDesc, _simpleSampler.ReleaseAndGetAddressOf());
	
	_directionalLight = std::make_unique<DirectionalLight>();
	_directionalLight->initialize(_device);

	_pointLight = std::make_unique<PointLight>();
	_pointLight->initialize(_device);

	_spotLight = std::make_unique<SpotLight>();
	_spotLight->initialize(_device);

	_skyLight = std::make_unique<SkyLight>();
	_skyLight->initialize(_device);

}

const Texture GraphicsResourceManager::loadTexture(const std::string & assetId) {

	if (_textures.count(assetId) > 0) {
		return _textures[assetId];
	}

	DirectX::ScratchImage image;
	DirectX::ScratchImage mipChain;
	DirectX::TexMetadata metaData;

	const std::string fullPath = assetId;
	int path_i = fullPath.find_first_of("/") + 1;
	const std::string fileName = fullPath.substr(path_i);
	path_i = fileName.find_last_of(".") + 1;
	const std::string fileExtension = fileName.substr(path_i);

	//char*からwchar_tに変換する
	const size_t cSize = fullPath.size() + 1;
	std::unique_ptr<wchar_t> wc = std::unique_ptr<wchar_t>(new wchar_t[cSize]);

	mbstowcs_s(NULL, wc.get(), cSize, fullPath.c_str(), _TRUNCATE);

	ComPtr<ID3D11ShaderResourceView> texture;

	//ファイルを読み込んでリソースビューを作成
	if (fileExtension == "png") {

		//ファイル名から画像を生成
		DirectX::LoadFromWICFile(wc.get(), DirectX::DDS_FLAGS_NONE, &metaData, image);

		//ミップマップ生成
		DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_BOX, 0, mipChain);

		//ミップマップ付きのイメージをシェーダーリソースビューにセット
		CreateShaderResourceView(_device.Get(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), texture.ReleaseAndGetAddressOf());
	}

	//BC_7ロード 700ms シェーダーリソースビュー生成 200ms
	if (fileExtension == "dds") {
		HRESULT hr = DirectX::LoadFromDDSFile(wc.get(), DirectX::DDS_FLAGS_NONE, &metaData, image);
		CreateShaderResourceView(_device.Get(), image.GetImages(), image.GetImageCount(), metaData, texture.ReleaseAndGetAddressOf());
	}

	_textures[assetId] = texture;

	return _textures[assetId];
}

RefPtr<RenderableObject> GraphicsResourceManager::loadRenderableObject(const std::string & assetId, const std::vector<std::string>& matFiles) {

	std::string fileAndMatNames = assetId;
	for (auto&& m : matFiles){
		fileAndMatNames += "_" + m;
	}

	//すでにロードされていたらキャッシュインスタンスを返す
	if (_renderableObjects.count(fileAndMatNames) > 0) {
		return _renderableObjects[fileAndMatNames].get();
	}

	auto& gameRenderer = GameRenderer::instance();
	std::unique_ptr<RenderableObject> result;

	//メッシュローダーでロード開始
	MeshLoader meshLoader(gameRenderer.device(), gameRenderer.deviceContext());
	meshLoader.load(assetId, matFiles);

	//メッシュタイプを取得して種類ごとの処理を実行
	auto& mesh = meshLoader.getMeshes();
	
	if (meshLoader.getType() == MeshType::Static) {

		StaticMesh* ptr = new StaticMesh(std::move(mesh));
		ptr->setUp(gameRenderer.device());
		result = std::unique_ptr<StaticMesh>(ptr);

	} else if (meshLoader.getType() == MeshType::Skeletal) {

		auto skeleton = meshLoader.loadSkeleton();
		result = std::make_unique<SkeletalMesh>(std::move(mesh), std::move(skeleton));
	}

	RefPtr<RenderableObject> resultPtr = result.get();

	_renderableObjects[fileAndMatNames] = std::move(result);

	return resultPtr;
}

RefPtr<StaticInstanceMesh> GraphicsResourceManager::loadStaticInstanceMesh(
	const std::string& assetId,
	const std::vector<std::string>& matFiles,
	const std::vector<Matrix4>& matrices,
	uint32 meshDrawOffset,
	uint32 matrixBufferOffset) {
	
	std::string fileAndMatNames = assetId;
	for (auto&& m : matFiles) {
		fileAndMatNames += "_" + m;
	}

	//すでにロードされていたらキャッシュインスタンスを返す
	if (_renderableObjects.count(fileAndMatNames) > 0) {
		return static_cast<StaticInstanceMesh*>(_renderableObjects[fileAndMatNames].get());
	}

	auto& gameRenderer = GameRenderer::instance();
	std::unique_ptr<StaticInstanceMesh> result;

	//メッシュローダーでロード開始
	MeshLoader meshLoader(gameRenderer.device(), gameRenderer.deviceContext());
	meshLoader.load(assetId, matFiles);

	result = std::make_unique<StaticInstanceMesh>(meshLoader.getMeshes());
	result->setUp(gameRenderer.device(), matrices, meshDrawOffset, matrixBufferOffset);
	_renderableObjects[fileAndMatNames] = std::move(result);
	
	return static_cast<StaticInstanceMesh*>(_renderableObjects[fileAndMatNames].get());
}

RefPtr<DirectionalLight> GraphicsResourceManager::getDirectionalLight() const {
	return _directionalLight.get();
}

RefPtr<SkyLight> GraphicsResourceManager::getSkyLight() const
{
	return _skyLight.get();
}

RefPtr<PointLight> GraphicsResourceManager::getPointLight() const
{
	return _pointLight.get();
}

RefPtr<SpotLight> GraphicsResourceManager::getSpotLight() const
{
	return _spotLight.get();
}

const ComPtr<ID3D11SamplerState>& GraphicsResourceManager::simpleSamplerState() const{
	return _simpleSampler;
}
