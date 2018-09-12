#include <Renderer/PostEffect/PostProcess.h>
#include <cmath>
#include <Renderer/RendererUtil.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/OrthoScreen.h>
#include <Renderer/Camera.h>
#include <Renderer/GraphicsBuffers.h>

struct PostProcessConstant {
	Matrix4 mtxViewProjInverse;
	Matrix4 mtxViewProj;
};

ComPtr<ID3D11Buffer> _postProcessConstant;
void PostProcess::initialize(ComPtr<ID3D11Device> device) {

	HRESULT hr;

	RendererUtil::createPixelShader("PostProcess.cso", _postProcessShader, device);
	RendererUtil::createPixelShader("GaussianBlur_ps.cso", _gaussianBlurShader, device);
	RendererUtil::createConstantBuffer(_gaussianConstantBuffer, sizeof(GaussBlurParam), device);

	//ブルーム用アルファブレンドステートを生成
	D3D11_BLEND_DESC BlendDesc;
	ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&BlendDesc, _blendState.GetAddressOf());

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
	hr = device->CreateSamplerState(&sampDesc, _linerSampler.ReleaseAndGetAddressOf());

	RendererUtil::createConstantBuffer(_postProcessConstant, sizeof(PostProcessConstant), device);
}

HRESULT PostProcess::setupRenderResource(ComPtr<ID3D11Device> device, uint16 width, uint16 height) {
	
	_width = width;
	_height = height;

	uint32 donwSampleWidth = _width / 2;
	uint32 donwSampleHeight = _height / 2;
	for (int i = 0; i < BLOOM_DOWN_SAMPLE; ++i) {

		_bloomDownSamples[i] = std::make_unique<RenderTarget>(device.Get(), DXGI_FORMAT_R16G16B16A16_FLOAT, donwSampleWidth, donwSampleHeight);
		_bloomDownSamples[i + BLOOM_DOWN_SAMPLE] = std::make_unique<RenderTarget>(device.Get(), DXGI_FORMAT_R16G16B16A16_FLOAT, donwSampleWidth, donwSampleHeight);

		donwSampleWidth >>= 1;
		donwSampleHeight >>= 1;
	}
	
	return S_OK;
}

void PostProcess::draw(ComPtr<ID3D11DeviceContext> deviceContext, RefPtr<Deferredbuffers> deferredBuffers, RefPtr<OrthoScreen> orthoScreen, RefPtr<Camera> camera) {
	
	const float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


	ID3D11SamplerState* oldSampler;
	deviceContext->PSGetSamplers(0, 1, &oldSampler);

	const float blendFactorOne[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	deviceContext->OMSetBlendState(_blendState.Get(), blendFactorOne, D3D11_DEFAULT_SAMPLE_MASK);
	deviceContext->PSSetSamplers(0, 1, _linerSampler.GetAddressOf());

	//Bloom用の縮小バッファ描画とガウシアンブラーをかける
	{
		uint32 donwSampleWidth = _width / 2;
		uint32 donwSampleHeight = _height / 2;
		float deviation = 3.0f;
		float multiply = 1.0f;

		//RTクリア
		for (int i = 0; i < BLOOM_DOWN_SAMPLE; ++i) {
			deviceContext->ClearRenderTargetView(_bloomDownSamples[i]->rtv(), clearColor);
			deviceContext->ClearRenderTargetView(_bloomDownSamples[i + BLOOM_DOWN_SAMPLE]->rtv(), clearColor);
		}

		//縮小バッファを切り替えながらブラー設定
		for (int i = 0; i < BLOOM_DOWN_SAMPLE; ++i) {

			ID3D11ShaderResourceView* srv = 0;

			if (i == 0) {
				srv = deferredBuffers->getRenderTarget(3)->srv();
			}
			else {
				srv = _bloomDownSamples[i - 1]->srv();
			}

			D3D11_VIEWPORT viewport;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = static_cast<float>(donwSampleWidth);
			viewport.Height = static_cast<float>(donwSampleHeight);
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			GaussBlurParam src(donwSampleWidth, donwSampleHeight, Vector2(1, 0));

			if (i == 0) {
				src.offset[15].w = 1.0f;
			}

			deviceContext->OMSetRenderTargets(1, _bloomDownSamples[i]->ppRtv(), 0);
			deviceContext->UpdateSubresource(_gaussianConstantBuffer.Get(), 0, 0, &src, 0, 0);
			deviceContext->PSSetConstantBuffers(0, 1, _gaussianConstantBuffer.GetAddressOf());
			deviceContext->RSSetViewports(1, &viewport);
			deviceContext->PSSetShader(_gaussianBlurShader.Get(), 0, 0);
			deviceContext->PSSetShaderResources(0, 1, &srv);
			deviceContext->Draw(4, 0);

			srv = _bloomDownSamples[i]->srv();
			src = GaussBlurParam(donwSampleWidth, donwSampleHeight, Vector2(0, 1));

			deviceContext->OMSetRenderTargets(1, _bloomDownSamples[i + BLOOM_DOWN_SAMPLE]->ppRtv(), 0);
			deviceContext->PSSetShaderResources(0, 1, &srv);
			deviceContext->UpdateSubresource(_gaussianConstantBuffer.Get(), 0, 0, &src, 0, 0);
			deviceContext->Draw(4, 0);

			donwSampleWidth >>= 1;
			donwSampleHeight >>= 1;
			multiply *= 2.0f;

		}
	}

	ID3D11ShaderResourceView* const downSampleBlurs[4] = {
		_bloomDownSamples[0 + BLOOM_DOWN_SAMPLE]->srv(),
		_bloomDownSamples[1 + BLOOM_DOWN_SAMPLE]->srv(),
		_bloomDownSamples[2 + BLOOM_DOWN_SAMPLE]->srv(),
		_bloomDownSamples[3 + BLOOM_DOWN_SAMPLE]->srv(),
	};

	deviceContext->PSSetSamplers(0, 1, _linerSampler.GetAddressOf());
	deviceContext->OMSetBlendState(0, blendFactor, D3D11_DEFAULT_SAMPLE_MASK);
	deferredBuffers->setViewPort(deviceContext);

	PostProcessConstant constant;
	constant.mtxViewProjInverse = camera->mtxProj.inverse().multiply(camera->mtxView.inverse()).transpose();
	constant.mtxViewProj = camera->mtxProj.transpose();
	deviceContext->UpdateSubresource(_postProcessConstant.Get(), 0, 0, &constant, 0, 0);

	//最終パスポストプロセス
	orthoScreen->setBackBuffer();
	deviceContext->PSSetShader(_postProcessShader.Get(), 0, 0);
	deviceContext->PSSetShaderResources(0, 1, deferredBuffers->getRenderTarget(3)->ppSrv());
	deviceContext->PSSetShaderResources(1, 4, downSampleBlurs);
	deviceContext->PSSetShaderResources(5, 1, deferredBuffers->getDepthStencilResource().GetAddressOf());
	deviceContext->PSSetShaderResources(6, 1, deferredBuffers->getRenderTarget(1)->ppSrv());
	deviceContext->PSSetConstantBuffers(0, 1, _postProcessConstant.GetAddressOf());
	orthoScreen->setOrthoScreenVertex();
	deviceContext->Draw(4, 0);

	deviceContext->PSSetSamplers(0, 1, &oldSampler);
}

GaussBlurParam::GaussBlurParam(uint32 width, uint32 height, Vector2 dir, float range) {

	float t = 0.0f;
	float d = range * range / 100.0f;
	for (int i = 0; i < 10; ++i) {
		float r = 1.0f + 2.0f* i;
		float e = -0.5f * (r * r) / d;
		float w = exp(e);
		offset[i].x = w;
		if (i > 0) { w *= 2.0f; }
		t += w;
	}

	for (int i = 0; i < 10; ++i) {
		offset[i].x /= t;
	}

	sampleCount = 19;
	if (dir.x == 1) {
		offset[15].x = 1.0f / (float)width;
	}
	else {
		offset[15].x = 1.0f / (float)height;
	}

	offset[15].y = dir.x;
	offset[15].z = dir.y;
}
