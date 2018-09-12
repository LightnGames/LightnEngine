#include <Renderer/PostEffect/SSAO.h>
#include <Renderer/RendererUtil.h>
#include <cmath>
#include <Renderer/RendererUtil.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/OrthoScreen.h>
#include <Renderer/Camera.h>
#include <Renderer/GraphicsBuffers.h>
#include <Renderer/PostEffect/PostProcess.h>

struct PostProcessConstant {
	Matrix4 mtxViewProjInverse;
	Matrix4 mtxViewProj;
};

void SSAO::initialize(ComPtr<ID3D11Device> device) {
	RendererUtil::createPixelShader("SSAO_ps.cso", _ssaoShader, device);
	RendererUtil::createPixelShader("BilateralFilter_ps.cso", _bilateralFilterShader, device);
	RendererUtil::createConstantBuffer(_postProcessConstantPP, sizeof(PostProcessConstant), device);
	RendererUtil::createConstantBuffer(_gaussianConstant, sizeof(GaussBlurParam), device);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MaxAnisotropy = 1;//ミップマップの遠近レベル
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, _linerSampler.ReleaseAndGetAddressOf());
}

HRESULT SSAO::setupRenderResource(ComPtr<ID3D11Device> device, uint16 width, uint16 height) {

	_width = width / 2;
	_height = height / 2;

	_ssaoRt = nullptr;
	_ssaoRt2 = nullptr;
	_ssaoRt = std::make_unique<RenderTarget>(device.Get(), DXGI_FORMAT_R8G8B8A8_UNORM, _width, _height);
	_ssaoRt2 = std::make_unique<RenderTarget>(device.Get(), DXGI_FORMAT_R8G8B8A8_UNORM, _width, _height);


	return S_OK;
}

void SSAO::draw(ComPtr<ID3D11DeviceContext> deviceContext, RefPtr<Deferredbuffers> deferredBuffers, RefPtr<OrthoScreen> orthoScreen, RefPtr<Camera> camera) {

	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

	
	D3D11_VIEWPORT oldVp;
	uint32 vpNum = 1;
	deviceContext->RSGetViewports(&vpNum, &oldVp);
	deviceContext->OMSetBlendState(0, blendFactor, D3D11_DEFAULT_SAMPLE_MASK);
	deviceContext->PSSetSamplers(0, 1, _linerSampler.GetAddressOf());

	D3D11_VIEWPORT ssaoVp;
	ssaoVp.Width = static_cast<float>(_width);
	ssaoVp.Height = static_cast<float>(_height);
	ssaoVp.MinDepth = 0.0f;
	ssaoVp.MaxDepth = 1.0f;
	ssaoVp.TopLeftX = 0.0f;
	ssaoVp.TopLeftY = 0.0f;
	deviceContext->RSSetViewports(1, &ssaoVp);

	orthoScreen->setOrthoScreenVertex();

	//SSAO
	{
		PostProcessConstant constant;
		constant.mtxViewProjInverse = camera->mtxProj.inverse().multiply(camera->mtxView.inverse()).transpose();
		constant.mtxViewProj = camera->mtxProj.transpose();
		deviceContext->UpdateSubresource(_postProcessConstantPP.Get(), 0, 0, &constant, 0, 0);

		deviceContext->ClearRenderTargetView(_ssaoRt->rtv(), clearColor);
		deviceContext->OMSetRenderTargets(1, _ssaoRt->ppRtv(), 0);
		deviceContext->PSSetShader(_ssaoShader.Get(), 0, 0);
		deviceContext->PSSetShaderResources(0, 1, deferredBuffers->getDepthStencilResource().GetAddressOf());
		deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getRenderTarget(1)->ppSrv());
		deviceContext->PSSetConstantBuffers(0, 1, _postProcessConstantPP.GetAddressOf());
		deviceContext->Draw(4, 0); 
	}

	//ブラーパス
	{
		GaussBlurParam src(_width, _height, Vector2(1, 0), 50);
		deviceContext->UpdateSubresource(_gaussianConstant.Get(), 0, 0, &src, 0, 0);

		deviceContext->ClearRenderTargetView(_ssaoRt2->rtv(), clearColor);
		deviceContext->OMSetRenderTargets(1, _ssaoRt2->ppRtv(), 0);
		deviceContext->PSSetShader(_bilateralFilterShader.Get(), 0, 0);
		deviceContext->PSSetShaderResources(0, 1, _ssaoRt->ppSrv());
		deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getDepthStencilResource().GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, _gaussianConstant.GetAddressOf());
		deviceContext->Draw(4, 0);

		src = GaussBlurParam(_width, _height, Vector2(0, 1), 50);
		deviceContext->UpdateSubresource(_gaussianConstant.Get(), 0, 0, &src, 0, 0);

		deviceContext->OMSetRenderTargets(1, _ssaoRt->ppRtv(), 0);
		deviceContext->PSSetShaderResources(0, 1, _ssaoRt2->ppSrv());
		deviceContext->PSSetShaderResources(1, 1, deferredBuffers->getDepthStencilResource().GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, _gaussianConstant.GetAddressOf());
		deviceContext->Draw(4, 0); 
	}

	deviceContext->OMSetRenderTargets(1, deferredBuffers->getRenderTarget(3)->ppRtv(), deferredBuffers->getDepthStencilView(true));
	deviceContext->OMSetBlendState(orthoScreen->_blendState.Get(), blendFactor, D3D11_DEFAULT_SAMPLE_MASK);
	deviceContext->RSSetViewports(1, &oldVp);

}

ID3D11ShaderResourceView** SSAO::ssaoResource() {
	return _ssaoRt->ppSrv();
}
