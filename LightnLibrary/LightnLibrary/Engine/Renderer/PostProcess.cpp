#include "PostProcess.h"
#include <cmath>
#include <Renderer/RendererUtil.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/OrthoScreen.h>

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
	device->CreateBlendState(&BlendDesc, _blendState.GetAddressOf());

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

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MipLODBias = 0;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.BorderColor[0] = 0.0f;
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;
	desc.BorderColor[3] = 0.0f;
	desc.MinLOD = 0; // -FLT_MAX
	desc.MaxLOD = D3D11_FLOAT32_MAX; // FLT_MAX

	hr = device->CreateSamplerState(&desc, _pointSampler.GetAddressOf());
}

HRESULT PostProcess::setupRenderResource(ComPtr<ID3D11Device> device, uint16 width, uint16 height) {
	
	_width = width;
	_height = height;

	uint32 donwSampleWidth = _width / 2;
	uint32 donwSampleHeight = _height / 2;
	for (int i = 0; i < BLOOM_DOWN_SAMPLE; ++i) {

		uint32 idxA = i;
		uint32 idxB = i + BLOOM_DOWN_SAMPLE;
		createRenderTargets(_bloomDownSampleTex[idxA], _bloomDownSampleRTV[idxA], _bloomDownSampleSRV[idxA], donwSampleWidth, donwSampleHeight, device);
		createRenderTargets(_bloomDownSampleTex[idxB], _bloomDownSampleRTV[idxB], _bloomDownSampleSRV[idxB], donwSampleWidth, donwSampleHeight, device);
		donwSampleWidth >>= 1;
		donwSampleHeight >>= 1;
	}
	
	return S_OK;
}

void PostProcess::draw(ComPtr<ID3D11DeviceContext> deviceContext, RefPtr<Deferredbuffers> deferredBuffers, RefPtr<OrthoScreen> orthoScreen) {
	
	const float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


	ID3D11SamplerState* oldSampler;
	deviceContext->PSGetSamplers(0, 1, &oldSampler);

	const float blendFactorOne[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	deviceContext->OMSetBlendState(_blendState.Get(), blendFactorOne, D3D11_DEFAULT_SAMPLE_MASK);
	deviceContext->PSSetSamplers(0, 1, _pointSampler.GetAddressOf());

	//Bloom用の縮小バッファ描画とガウシアンブラーをかける
	{
		uint32 donwSampleWidth = _width / 2;
		uint32 donwSampleHeight = _height / 2;
		float deviation = 2.5f;
		float multiply = 1.0f;

		//RTクリア
		for (int i = 0; i < BLOOM_DOWN_SAMPLE; ++i) {
			deviceContext->ClearRenderTargetView(_bloomDownSampleRTV[i].Get(), clearColor);
			deviceContext->ClearRenderTargetView(_bloomDownSampleRTV[i + BLOOM_DOWN_SAMPLE].Get(), clearColor);
		}

		//縮小バッファを切り替えながらブラー設定
		for (int i = 0; i < BLOOM_DOWN_SAMPLE; ++i) {

			ID3D11ShaderResourceView* srv = 0;

			if (i == 0) {
				srv = deferredBuffers->_shaderResourceViewArray[3].Get();
			}
			else {
				srv = _bloomDownSampleSRV[i - 1].Get();
			}

			D3D11_VIEWPORT viewport;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = static_cast<float>(donwSampleWidth);
			viewport.Height = static_cast<float>(donwSampleHeight);
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			GaussBlurParam src = CalcBlurParam(donwSampleWidth, donwSampleHeight, Vector2(1, 0), deviation, multiply);

			deviceContext->OMSetRenderTargets(1, _bloomDownSampleRTV[i].GetAddressOf(), 0);
			deviceContext->UpdateSubresource(_gaussianConstantBuffer.Get(), 0, 0, &src, 0, 0);
			deviceContext->PSSetConstantBuffers(0, 1, _gaussianConstantBuffer.GetAddressOf());
			deviceContext->RSSetViewports(1, &viewport);
			deviceContext->PSSetShader(_gaussianBlurShader.Get(), 0, 0);
			deviceContext->PSSetShaderResources(0, 1, &srv);
			deviceContext->Draw(4, 0);

			srv = _bloomDownSampleSRV[i].Get();
			src = CalcBlurParam(donwSampleWidth, donwSampleHeight, Vector2(0, 1), deviation, multiply);

			deviceContext->OMSetRenderTargets(1, _bloomDownSampleRTV[i + BLOOM_DOWN_SAMPLE].GetAddressOf(), 0);
			deviceContext->PSSetShaderResources(0, 1, &srv);
			deviceContext->UpdateSubresource(_gaussianConstantBuffer.Get(), 0, 0, &src, 0, 0);
			deviceContext->Draw(4, 0);

			donwSampleWidth >>= 1;
			donwSampleHeight >>= 1;
			multiply *= 2.0f;

		}
	}

	ID3D11ShaderResourceView* downSampleBlurs[3] = {
		_bloomDownSampleSRV[0 + BLOOM_DOWN_SAMPLE].Get(),
		_bloomDownSampleSRV[1 + BLOOM_DOWN_SAMPLE].Get(),
		_bloomDownSampleSRV[2 + BLOOM_DOWN_SAMPLE].Get(),
	};

	deviceContext->PSSetSamplers(0, 1, _linerSampler.GetAddressOf());
	deviceContext->OMSetBlendState(0, blendFactor, D3D11_DEFAULT_SAMPLE_MASK);
	deferredBuffers->setViewPort(deviceContext);

	//最終パスポストプロセス
	orthoScreen->setBackBuffer();
	deviceContext->PSSetShader(_postProcessShader.Get(), 0, 0);
	deviceContext->PSSetShaderResources(0, 1, deferredBuffers->_shaderResourceViewArray[3].GetAddressOf());
	deviceContext->PSSetShaderResources(1, 3, downSampleBlurs);
	orthoScreen->setOrthoScreenVertex();
	deviceContext->Draw(4, 0);

	deviceContext->PSSetSamplers(0, 1, &oldSampler);
}

inline GaussBlurParam PostProcess::CalcBlurParam(uint32 width, uint32 height, Vector2 dir, float deviation, float multiply) {
	
	GaussBlurParam result;
	result.sampleCount = 15;
	auto tu = 1.0f / float(width);
	auto tv = 1.0f / float(height);

	result.offset[0].z = GaussianDistribution(Vector2(0.0f, 0.0f), deviation)*multiply;
	auto total_weight = result.offset[0].z;

	result.offset[0].x = 0.0f;
	result.offset[0].y = 0.0f;

	for (auto i = 1; i < 8; ++i) {
		result.offset[i].x = dir.x * i * tu;
		result.offset[i].y = dir.y * i * tv;
		result.offset[i].z = GaussianDistribution(Vector2(dir.x*i, dir.y * i), deviation)*multiply;
		total_weight += result.offset[i].z * 2.0f;
	}

	for (auto i = 0; i < 8; ++i) {
		result.offset[i].z /= total_weight;
	}
	for (auto i = 8; i < 15; ++i) {
		result.offset[i].x = -result.offset[i - 7].x;
		result.offset[i].y = -result.offset[i - 7].y;
		result.offset[i].z = result.offset[i - 7].z;
	}

	return result;
}

inline float PostProcess::GaussianDistribution(const Vector2 & pos, float rho) {
	return exp(-(pos.x * pos.x + pos.y * pos.y) / (2.0f * rho * rho));
}

void PostProcess::createRenderTargets(ComPtr<ID3D11Texture2D>& tex, ComPtr<ID3D11RenderTargetView>& rtv, ComPtr<ID3D11ShaderResourceView>& srv, uint32 width, uint32 height, ComPtr<ID3D11Device> device) {

	HRESULT result;
	const DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	//レンダーターゲットを生成
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, 0, tex.ReleaseAndGetAddressOf());

	//レンダーターゲットビューの生成
	D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
	ZeroMemory(&renderDesc, sizeof(renderDesc));
	renderDesc.Format = format;
	renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(tex.Get(), &renderDesc, rtv.ReleaseAndGetAddressOf());

	//シェーダーリソースビューの生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	ZeroMemory(&shaderDesc, sizeof(shaderDesc));
	shaderDesc.Format = format;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(tex.Get(), &shaderDesc, srv.ReleaseAndGetAddressOf());

}
