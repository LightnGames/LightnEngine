#include "StaticMesh.h"
#include <Renderer/RendererSettings.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/SceneRendererManager.h>

StaticMesh::StaticMesh(const LocalMesh& meshes) :_meshes{ meshes } {
}

void StaticMesh::setUp(ComPtr<ID3D11Device> device) {
	
}

void StaticMesh::draw(const DrawSettings& drawSettings, const Matrix4& worldMatrix) {

	auto deviceContext = drawSettings.deviceContext;

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(worldMatrix, drawSettings.camera);

	drawMesh(deviceContext, static_cast<void*>(&constantBuffer), sizeof(MeshVertex));
}

void StaticMesh::drawDepth(const DrawSettings & drawSettings, const Matrix4 & worldMatrix)
{
	ComPtr<ID3D11DeviceContext> deviceContext = drawSettings.deviceContext;

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(worldMatrix, drawSettings.camera);

	//頂点バッファをセット
	const UINT stride = sizeof(MeshVertex);
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);

	//マテリアルの数だけループ
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];
		deviceContext->VSSetShader(material->pVertexShader.Get(), NULL, 0);
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, material->pConstantBuffer.GetAddressOf());
		deviceContext->DrawIndexed(material->faceCount * 3, 0, 0);
	}
}

void StaticMesh::drawMesh(ComPtr<ID3D11DeviceContext> deviceContext, const RefPtr<void>& constantBuffer, const UINT vertexBufferSize) {

	//頂点バッファをセット
	const UINT stride = vertexBufferSize;
	const UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _meshes.vertexBuffer.GetAddressOf(), &stride, &offset);
	
	//ポリゴンの描画ルールをセット
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//マテリアルの数だけループ
	for (UINT j = 0; j < _meshes.materialSlots.size(); ++j) {

		const auto& material = _meshes.materialSlots[j];

		//使用シェーダーをセット
		deviceContext->VSSetShader(material->pVertexShader.Get(), 0, 0);
		deviceContext->PSSetShader(material->pPixelShader.Get(), 0, 0);

		//インデックスバッファをセット
		deviceContext->IASetIndexBuffer(material->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//頂点インプットレイアウトをセット
		deviceContext->IASetInputLayout(material->pVertexLayout.Get());

		//コンスタントバッファー内容更新
		deviceContext->UpdateSubresource(material->pConstantBuffer.Get(), 0, NULL, constantBuffer.get(), 0, 0);

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

		//テクスチャサンプラーをセット
		deviceContext->PSSetSamplers(0, 1, material->pSamplerLiner.GetAddressOf());

		//描画
		deviceContext->DrawIndexed(material->faceCount * 3, 0, 0);

	}
}

RefPtr<MaterialData> StaticMesh::material(int index) {
	return _meshes.materialSlots[index].get();
}
