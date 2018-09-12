#include <Renderer/Light/SkyLight.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/RendererSettings.h>
#include <ThirdParty/ImGui/imgui.h>

void SkyLight::initialize(ComPtr<ID3D11Device>& device) {
	RendererUtil::createConstantBuffer(_lightBuffer, sizeof(SkyLightType), device);
	initializeLight(device, "DeferredLight_vs.cso", "DeferredSkyLight_ps.cso");
}

void SkyLight::draw(const DrawSettings & settings, RefPtr<LightComponent>& lightComponent) {

	auto deviceContext = settings.deviceContext;
	auto camera = settings.camera;

	ImGui::Begin("SkyLight");
	static Vector3 lightColor = Vector3::one;
	static float diffuseIntensity = 1.5f;
	static float roughnessIntensity = 0.25f;

	ImGui::ColorEdit3("LightColor", &lightColor.x);
	ImGui::SliderFloat("Diffuse Intensity", &diffuseIntensity, 0.0f, 3.0f);
	ImGui::SliderFloat("Roughness Intensity", &roughnessIntensity, 0.0f, 1.0f);
	ImGui::End();

	SkyLightType lightBuffer;
	lightBuffer.color = lightColor;
	lightBuffer.intensity = Vector4(diffuseIntensity, roughnessIntensity, 0, 0);
	lightBuffer.mtxViewProjInv = Matrix4::multiply(camera->mtxProj.inverse(), camera->mtxView.inverse()).transpose();
	lightBuffer.cameraPosition = camera->position;

	deviceContext->UpdateSubresource(_lightBuffer.Get(), 0, 0, &lightBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _lightBuffer.GetAddressOf());


	if (RendererSettings::skyBox.Get() != nullptr) {
		deviceContext->PSSetShaderResources(4, 1, RendererSettings::skyBox.GetAddressOf());
	}

	Light::draw(settings, lightComponent);
}
