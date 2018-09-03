#include "DirectionalLight.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <ThirdParty/ImGui/imgui.h>
#include <Components/LightComponent.h>

#include <Renderer/SceneRendererManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RenderableEntity.h>
#include <Renderer/GameRenderer.h>
#include <Renderer/StaticInstancedMeshRenderer.h>
#include <ThirdParty/ImGui/imgui.h>

DirectionalLight::DirectionalLight() {
}

DirectionalLight::~DirectionalLight() {
}

struct DirectionalShadowConstant {
	Matrix4 lightMtxView;
	Matrix4 lightMtxProj;
};

void DirectionalLight::initialize(ComPtr<ID3D11Device>& device) {
	RendererUtil::createConstantBuffer(_lightBuffer, sizeof(DirectionalLightType), device);
	initializeLight(device, "DeferredLight_vs.cso", "DeferredDirectionalLight_ps.cso");

}

void DirectionalLight::draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent) {

	auto deviceContext = settings.deviceContext;
	auto camera = settings.camera;

	ImGui::Begin("DirectionalLight");
	static float pitchDir = 40.0;
	static float yawDir = 40.0;
	static Vector3 lightColor = Vector3::one;
	static float lightIntensity = 5.0f;

	ImGui::ColorEdit3("Color", &lightColor.x);
	ImGui::SliderFloat("Pitch", &pitchDir, 0.0f, 360.0f);
	ImGui::SliderFloat("Yaw", &yawDir, 0.0f, 360.0f);
	ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 100.0f);
	ImGui::End();

	Matrix4 viewOffset;
	viewOffset = Matrix4::translateXYZ(Quaternion::rotVector(settings.camera->rotation, Vector3::forward*30));

	DirectionalLightType lightBuffer;
	ID3D11RenderTargetView* beforeViews = 0;
	ID3D11DepthStencilView* beforeDepthStencilView = 0;
	ID3D11DepthStencilState* beforeDepthState = 0;
	ID3D11BlendState* beforeBlendState = 0;
	D3D11_VIEWPORT beforeVp;
	uint32 oldVpNum = 1;
	deviceContext->OMGetRenderTargets(1, &beforeViews, &beforeDepthStencilView);
	deviceContext->OMGetDepthStencilState(&beforeDepthState, 0);
	deviceContext->OMGetBlendState(&beforeBlendState, 0, 0);
	deviceContext->RSGetViewports(&oldVpNum, &beforeVp);

	if (lightComponent->isEnableShadow()) {

		//シャドウパラメータ
		RefPtr<ShadowResource> resource = lightComponent->getShadowResource();
		ID3D11DepthStencilView* depthView = resource->_depthStencilView.Get();
		ID3D11ShaderResourceView* depthSRV = resource->_depthStencilSRV.Get();
		ID3D11SamplerState* shadowSampler = resource->_shadowSampler.Get();
		D3D11_VIEWPORT& shadowVp = resource->shadowVp;

		const uint32 scale = resource->scale;
		const float nearZ = resource->nearZ;
		const float farZ = resource->farZ;

		//シャドウデプス描画用ステートに変更
		deviceContext->OMSetRenderTargets(0, 0, depthView);
		deviceContext->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		deviceContext->OMSetDepthStencilState(0, 0);
		deviceContext->OMSetBlendState(0, 0, 0xffffffff);
		deviceContext->RSSetViewports(1, &shadowVp);


		//シャドウデプス描画用設定を作成
		DrawSettings lightSetting = settings;
		Camera lightCamera;
		lightCamera.position = camera->position + Vector3::up * 10;
		lightCamera.rotation = Quaternion::euler({ pitchDir,yawDir,0 });
		const Matrix4 mtxView =Matrix4::matrixFromQuaternion(lightCamera.rotation).multiply(Matrix4::translateXYZ(lightCamera.position)).multiply(viewOffset);
		lightCamera.mtxView = mtxView.inverse();
		lightCamera.mtxProj = Matrix4::orthographicProjectionLH(scale, scale, nearZ, farZ, 0.1f);
		lightSetting.camera = &lightCamera;

		//シャドウカメラからカリング情報を再構築
		StaticInstancedMeshRenderer::instance().clearCullingBufferShadow(deviceContext, lightCamera);

		//シャドウデプスを描画
		auto& sceneRendererManager = SceneRendererManager::instance();
		for (auto&& sm : sceneRendererManager.renderableEntities()) {
			sm->drawDepth(lightSetting);
		}

		ImGui::Begin("shadow");
		ImGui::Image(depthSRV, { 200,200 });
		ImGui::End();

		//シャドウデプスの描画が終わったので設定を元に戻す
		GameRenderer::instance().setOrthoScreenVertex();
		deviceContext->OMSetRenderTargets(1, &beforeViews, beforeDepthStencilView);
		deviceContext->OMSetDepthStencilState(beforeDepthState, 1);
		
		const float factor[4] = { 0,0,0,0 };
		deviceContext->OMSetBlendState(beforeBlendState, factor, D3D11_DEFAULT_SAMPLE_MASK);
		deviceContext->RSSetViewports(1, &beforeVp);

		//シャドウで使用する行列をセット
		lightBuffer.mtxViewProjInv = Matrix4::multiply(camera->mtxProj.inverse(), camera->mtxView.inverse()).transpose();
		lightBuffer.mtxShadow = lightCamera.mtxView.multiply(lightCamera.mtxProj).multiply(Matrix4::textureBias).transpose();

		//シャドウ用リソースをシェーダーにセット
		deviceContext->PSSetShaderResources(4, 1, &depthSRV);
		deviceContext->PSSetSamplers(1, 1, &shadowSampler);
	}

	Vector3 lightDirection = Vector3::forward;
	lightDirection = Quaternion::rotVector(Quaternion::euler({ pitchDir,yawDir,0 }), lightDirection);
	lightBuffer.direction = Vector3::normalize(-lightDirection);
	lightBuffer.lightColor = lightColor * lightIntensity;
	lightBuffer.cameraPosition = camera->position;

	deviceContext->UpdateSubresource(_lightBuffer.Get(), 0, 0, &lightBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _lightBuffer.GetAddressOf());

	Light::draw(settings, lightComponent);

	deviceContext->OMSetRenderTargets(1, &beforeViews, beforeDepthStencilView);

}
