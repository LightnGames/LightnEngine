#include <Renderer/Mesh/StaticInstanceMesh.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererSettings.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/StaticInstancedMeshRenderer.h>

StaticInstanceMesh::StaticInstanceMesh(const LocalMesh & meshes) :_meshes{ meshes } {
}

void StaticInstanceMesh::setUp(
	ComPtr<ID3D11Device> device,
	const std::vector<Matrix4>& matrices,
	uint32 meshDrawOffset,
	uint32 matrixBufferOffset) {

	_meshDrawOffset = meshDrawOffset;
	_matrixBufferOffset = matrixBufferOffset;

	//カリング対象のバウンディングボックス配列を生成
	std::vector<BoundingBoxInfo> boundingBoxes;
	{
		boundingBoxes.reserve(matrices.size());
		_cullingBoxes.reserve(matrices.size());

		for (auto&& m : matrices) {
			AABB transformdAABB = _meshes.boundingBox.createTransformMatrix(m);
			boundingBoxes.emplace_back(BoundingBoxInfo(transformdAABB.size(), Matrix4::transpose(m)));

			Vector3 p(m.m[3][0], m.m[3][1], m.m[3][2]);
			transformdAABB.translate(p);
			_cullingBoxes.emplace_back(transformdAABB);
		}

		//インスタンス数を保持
		_instanceCount = static_cast<uint32>(boundingBoxes.size());

		//GPUで4スレッドで動作するため不足分を詰めておく
		uint32 size = THREAD_NUM - (boundingBoxes.size() % THREAD_NUM);
		if (size < THREAD_NUM) {
			for (uint32 i = 0; i < size; ++i) {
				boundingBoxes.emplace_back(BoundingBoxInfo(Vector3::zero, Matrix4::scaleXYZ(0, 0, 0)));
			}
		}

		//不足分を詰めた後の数
		_paddingedCount = static_cast<uint32>(boundingBoxes.size());

		//0に全体メッシュのオフセット 1にスレッド数の倍数に合わせる前の数 2にマテリアル数
		boundingBoxes[0].extent.w = static_cast<float>(meshDrawOffset);
		boundingBoxes[1].extent.w = static_cast<float>(_instanceCount);
		boundingBoxes[2].extent.w = static_cast<float>(_meshes.materialSlots.size());
	}

	//GPUバッファを生成
	{
		//引数に渡すバウンディングボックスリストバッファ
		D3D11_BUFFER_DESC cb;
		ZeroMemory(&cb, sizeof(cb));
		cb.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		cb.ByteWidth = _paddingedCount * sizeof(BoundingBoxInfo);
		cb.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		cb.StructureByteStride = sizeof(BoundingBoxInfo);

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = boundingBoxes.data();

		HRESULT hr = device->CreateBuffer(&cb, &initData, _boundingBoxListBuffer.ReleaseAndGetAddressOf());
		hr = device->CreateShaderResourceView(_boundingBoxListBuffer.Get(), nullptr, _boundingBoxListSRV.ReleaseAndGetAddressOf());

		const uint32 m[4] = { _matrixBufferOffset, 0, 0, 0 };
		RendererUtil::createConstantBuffer(_meshDrawOffsetConstantBuffer, sizeof(uint32) * 4, device, m);
	}
}

void StaticInstanceMesh::draw(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData) {
	auto deviceContext = drawSettings.deviceContext;

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(Matrix4::identity, drawSettings.camera);

	//頂点バッファをセット
	const UINT stride = sizeof(MeshVertex);
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);
	deviceContext->VSSetShaderResources(0, 1, instanceData->instanceMatrixSRV.GetAddressOf());
	deviceContext->VSSetConstantBuffers(1, 1, _meshDrawOffsetConstantBuffer.GetAddressOf());

	//ポリゴンの描画ルールをセット
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//マテリアルの数だけループ
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];

		//使用シェーダーをセット
		deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);
		deviceContext->PSSetShader(material->pPixelShader.Get(), NULL, 0);

		//インデックスバッファをセット
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//頂点インプットレイアウトをセット
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());

		//コンスタントバッファー内容更新
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);

		//コンスタントバッファーを使うシェーダーにセット
		deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());

		//CubeMapテクスチャをセット
		int globalTextureCount = 1;
		if (RendererSettings::skyBox.Get() != nullptr) {
			deviceContext->PSSetShaderResources(0, 1, RendererSettings::skyBox.GetAddressOf());
		}

		//テクスチャリソースをセット(シェーダー内の並び順と一致する必要あり)
		for (UINT k = 0; k < material->textureCount; ++k) {
			deviceContext->PSSetShaderResources(k + globalTextureCount, 1, material->ppTextures[k].GetAddressOf());
		}

		deviceContext->RSSetState(GraphicsResourceManager::instance().rasterState(material->cullMode));

		//テクスチャサンプラーをセット
		deviceContext->PSSetSamplers(0, 1, material->pSamplerLiner.GetAddressOf());

		//描画
		deviceContext->DrawIndexedInstancedIndirect(instanceData->indtsnceDrawListBuffer.Get(), (_meshDrawOffset + j) * 20);

	}
}


#include <Renderer/SceneRendererManager.h>
void StaticInstanceMesh::drawDepth(const DrawSettings& drawSettings, RefPtr<StaticInstanceMeshData> instanceData) {
	auto deviceContext = drawSettings.deviceContext;

	for (auto&& b : _cullingBoxes) {
		//SceneRendererManager::debugDrawBox(b.center(), b.size());
	}

	//カリング
	{
		uint32 offset = _matrixBufferOffset;
		deviceContext->CSSetShaderResources(0, 1, _boundingBoxListSRV.GetAddressOf());
		deviceContext->CSSetConstantBuffers(0, 1, instanceData->frustumCullingBuffer.GetAddressOf());
		deviceContext->CSSetUnorderedAccessViews(0, 1, instanceData->instanceMatrixUAV.GetAddressOf(), &offset);
		deviceContext->CSSetUnorderedAccessViews(1, 1, instanceData->instanceDrawListUAV.GetAddressOf(), 0);
		deviceContext->CSSetShader(instanceData->cullingShader.Get(), nullptr, 0);
		deviceContext->Dispatch(_paddingedCount / THREAD_NUM, 1, 1);

		ID3D11UnorderedAccessView* ppUAViewNULL[2] = { nullptr, nullptr };
		ID3D11ShaderResourceView* ppSRVNULL[1] = { nullptr };
		ID3D11Buffer* ppCBNULL[1] = { nullptr };
		deviceContext->CSSetShader(nullptr, nullptr, 0);
		deviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);
		deviceContext->CSSetUnorderedAccessViews(0, 2, ppUAViewNULL, nullptr);
		deviceContext->CSSetShaderResources(0, 1, ppSRVNULL);
	}

	//描画
	{
		MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(Matrix4::identity, drawSettings.camera);

		//頂点バッファをセット
		const UINT stride = sizeof(MeshVertex);
		const UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);
		deviceContext->VSSetShaderResources(0, 1, instanceData->instanceMatrixSRV.GetAddressOf());
		deviceContext->VSSetConstantBuffers(1, 1, _meshDrawOffsetConstantBuffer.GetAddressOf());

		//ポリゴンの描画ルールをセット
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//マテリアルの数だけループ
		for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

			const auto& material = _meshes.materialSlots[j];

			//アルファがMaskedならアルベドのアルファを参照する必要があるのでそのテクスチャをセット
			if (material->alphaType == 1) {
				deviceContext->PSSetShader(GraphicsResourceManager::instance().simpleMaskedDepthShader(), 0, 0);
				deviceContext->PSSetShaderResources(0, 1, material->ppTextures[0].GetAddressOf());
			}

			deviceContext->RSSetState(GraphicsResourceManager::instance().rasterState(material->cullMode));
			deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);
			deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->IASetInputLayout(material->pVertexLayout.Get());
			deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);
			deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());
			deviceContext->DrawIndexedInstancedIndirect(instanceData->indtsnceDrawListBuffer.Get(), (_meshDrawOffset + j) * 20);

			deviceContext->PSSetShader(0, 0, 0);
		}
	}
}

RefPtr<const LocalMesh> StaticInstanceMesh::meshInfo() const {
	return &_meshes;
}

RefPtr<MaterialData> StaticInstanceMesh::material(int index) const {
	return _meshes.materialSlots[index].get();
}


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

	HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, terrainTex.GetAddressOf());
	assert(SUCCEEDED(hr) && "テクスチャ生成に失敗");

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = format;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(terrainTex.Get(), &shaderDesc, terrainSRV.GetAddressOf());
	assert(SUCCEEDED(hr) && "SRV生成に失敗");
}

void TerrainMesh::draw(const DrawSettings & drawSettings, RefPtr<StaticInstanceMeshData> instanceData) {
	ImGui::Begin("terrain");
	ImGui::Image(terrainSRV.Get(), ImVec2(200, 200));
	ImGui::End();
	
	auto deviceContext = drawSettings.deviceContext;
	deviceContext->VSSetShaderResources(1, 1, terrainSRV.GetAddressOf());
	deviceContext->VSSetSamplers(0, 1, GraphicsResourceManager::instance().simpleSamplerState().GetAddressOf());

	StaticInstanceMesh::draw(drawSettings, instanceData);
	
}

void TerrainMesh::drawDepth(const DrawSettings & drawSettings, RefPtr<StaticInstanceMeshData> instanceData) {
	
	auto deviceContext = drawSettings.deviceContext;
	deviceContext->VSSetShaderResources(1, 1, terrainSRV.GetAddressOf());
	deviceContext->VSSetSamplers(0, 1, GraphicsResourceManager::instance().simpleSamplerState().GetAddressOf());

	StaticInstanceMesh::drawDepth(drawSettings, instanceData);
}
