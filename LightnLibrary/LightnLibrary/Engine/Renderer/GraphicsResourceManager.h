#pragma once

#include <Util/Singleton.h>
#include <Util/ComPtr.h>
#include <Util/Type.h>
#include <d3d11.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <LMath.h>

class StaticInstanceMesh;
class RenderableObject;
class DirectionalLight;
class SkyLight;
class PointLight;
class SpotLight;

using Texture = ComPtr<ID3D11ShaderResourceView>;
using RenderableObjectPtr = std::unique_ptr<RenderableObject>;

class GraphicsResourceManager :public Singleton<GraphicsResourceManager>{

public:

	GraphicsResourceManager();

	~GraphicsResourceManager();

	//初期化
	void initialize(ComPtr<ID3D11Device> device);

	//テクスチャをロード
	const Texture loadTexture(const std::string& assetId);

	//描画オブジェクトを取得
	RefPtr<RenderableObject> loadRenderableObject(const std::string& assetId, const std::vector<std::string>& matFiles);

	RefPtr<StaticInstanceMesh> loadStaticInstanceMesh(
		const std::string& assetId,
		const std::vector<std::string>& matFiles,
		const std::vector<Matrix4>& matrices,
		uint32 meshId,
		uint32 matrixBufferOffset);

	//ディレクショナルライトを取得
	RefPtr<DirectionalLight> getDirectionalLight() const;

	//スカイライトを取得
	RefPtr<SkyLight> getSkyLight() const;

	//ポイントライトを取得
	RefPtr<PointLight> getPointLight() const;

	//スポットライトを取得
	RefPtr<SpotLight> getSpotLight() const;

	//シンプルサンプラを取得
	const ComPtr<ID3D11SamplerState>& simpleSamplerState() const;

	ID3D11PixelShader* simpleMaskedDepthShader();
	ID3D11RasterizerState* rasterState(D3D11_CULL_MODE mode);

private:

	std::unordered_map<std::string, Texture> _textures;
	std::unordered_map<std::string, RenderableObjectPtr> _renderableObjects;

	std::unique_ptr<DirectionalLight> _directionalLight;
	std::unique_ptr<SkyLight> _skyLight;
	std::unique_ptr<PointLight> _pointLight;
	std::unique_ptr<SpotLight> _spotLight;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11SamplerState> _simpleSampler;
	ComPtr<ID3D11PixelShader> _simpleMaskedDepthShader;
	ComPtr<ID3D11RasterizerState> _dualRaster;
};