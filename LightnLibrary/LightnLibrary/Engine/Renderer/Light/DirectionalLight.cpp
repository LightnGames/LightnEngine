#include "DirectionalLight.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <ThirdParty/ImGui/imgui.h>
#include <Components/LightComponent.h>

DirectionalLight::DirectionalLight() {
}

DirectionalLight::~DirectionalLight() {
}

void DirectionalLight::initialize(ComPtr<ID3D11Device>& device) {
	RendererUtil::createConstantBuffer(_lightBuffer, sizeof(DirectionalLightType), device);
	initializeLight(device, "DeferredLight_vs.cso", "DeferredDirectionalLight_ps.cso");
}

void DirectionalLight::draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent) {

	auto deviceContext = settings.deviceContext;

	ImGui::Begin("DirectionalLight");
	static float pitchDir = 25.0;
	static float yawDir = 40.0;
	static Vector3 lightColor = Vector3::one;
	static float lightIntensity = 1.0f;

	ImGui::ColorEdit3("Color", &lightColor.x);
	ImGui::SliderFloat("Pitch", &pitchDir, 0.0f, 360.0f);
	ImGui::SliderFloat("Yaw", &yawDir, 0.0f, 360.0f);
	ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 30.0f);
	ImGui::End();

	DirectionalLightType lightBuffer;

	Vector3 lightDirection = Vector3::forward;
	lightDirection = Quaternion::rotVector(Quaternion::euler({ pitchDir,yawDir,0 }), lightDirection);
	lightBuffer.direction = Vector3::normalize(-lightDirection);
	lightBuffer.lightColor = lightColor * lightIntensity;

	deviceContext->UpdateSubresource(_lightBuffer.Get(), 0, NULL, &lightBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _lightBuffer.GetAddressOf());

	Light::draw(settings, lightComponent);

}
