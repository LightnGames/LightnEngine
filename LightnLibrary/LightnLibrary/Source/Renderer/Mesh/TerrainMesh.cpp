#include <Renderer/Mesh/TerrainMesh.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Mesh/SkyBox.h>
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/StaticInstancedMeshRenderer.h>
#include <IO/BinaryLoader.h>
#include <ThirdParty/ImGui/imgui.h>

void TerrainMesh::setUp(ComPtr<ID3D11Device> device, const std::vector<Matrix4>& matrices, uint32 meshDrawOffset, uint32 matrixBufferOffset) {
	StaticInstanceMesh::setUp(device, matrices, meshDrawOffset, matrixBufferOffset);

	DXGI_FORMAT format = DXGI_FORMAT_R16_UNORM;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = 513;
	textureDesc.Height = 513;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	BinaryLoader terrainResource("Scene/Terrain.raw");

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = terrainResource.data();
	initData.SysMemPitch = 513 * 2;

	HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, _heightTexture.GetAddressOf());
	assert(SUCCEEDED(hr) && "テクスチャ生成に失敗");

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = format;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(_heightTexture.Get(), &shaderDesc, _heightSRV.GetAddressOf());
	assert(SUCCEEDED(hr) && "SRV生成に失敗");
}

void TerrainMesh::draw(const DrawSettings & drawSettings, RefPtr<StaticInstanceMeshData> instanceData) {
	ImGui::Begin("terrain");
	ImGui::Image(_heightSRV.Get(), ImVec2(200, 200));
	ImGui::End();

	auto deviceContext = drawSettings.deviceContext;
	deviceContext->VSSetShaderResources(1, 1, _heightSRV.GetAddressOf());
	deviceContext->VSSetSamplers(0, 1, GraphicsResourceManager::instance().simpleSamplerState().GetAddressOf());

	StaticInstanceMesh::draw(drawSettings, instanceData);

}

void TerrainMesh::drawDepth(const DrawSettings & drawSettings, RefPtr<StaticInstanceMeshData> instanceData) {

	auto deviceContext = drawSettings.deviceContext;
	deviceContext->VSSetShaderResources(1, 1, _heightSRV.GetAddressOf());
	deviceContext->VSSetSamplers(0, 1, GraphicsResourceManager::instance().simpleSamplerState().GetAddressOf());

	StaticInstanceMesh::drawDepth(drawSettings, instanceData);
}
