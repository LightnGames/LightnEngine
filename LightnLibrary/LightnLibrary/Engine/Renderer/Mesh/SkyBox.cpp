#include "SkyBox.h"

#include <Renderer/RendererSettings.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererUtil.h>

SkyBox::SkyBox(const LocalMesh & meshes, ComPtr<ID3D11Device> device) :StaticMesh(meshes) {

	HRESULT hr;
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	//ステンシル書き込み用
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&dsDesc, _stencilWriteState.ReleaseAndGetAddressOf());

	//ステンシル読み込み用
	dsDesc.DepthEnable = false;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	hr = device->CreateDepthStencilState(&dsDesc, _stencilReadState.ReleaseAndGetAddressOf());

}

void SkyBox::drawStencil(const DrawSettings & drawSettings) {

	auto deviceContext = drawSettings.deviceContext;
	deviceContext->OMSetDepthStencilState(_stencilWriteState.Get(), 1);
	deviceContext->PSSetShader(nullptr, 0, 0);

	//DepthPassと同じ描画ルールでステンシルを描画(RenderTarget&ピクセルシェーダー不要)
	StaticMesh::drawDepth(drawSettings, Matrix4::identity);
}

void SkyBox::draw(const DrawSettings & drawSettings) {

	auto deviceContext = drawSettings.deviceContext;
	deviceContext->OMSetDepthStencilState(_stencilReadState.Get(), 1);

	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(Matrix4::identity, drawSettings.camera);

	StaticMesh::drawMesh(deviceContext, &constantBuffer, sizeof(MeshVertex));
}
