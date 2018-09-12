#include <Renderer/Light/PointLight.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <ThirdParty/ImGui/imgui.h>
#include <Component/LightComponent.h>

PointLight::PointLight() {
}

PointLight::~PointLight() {
}

void PointLight::initialize(ComPtr<ID3D11Device>& device) {
	RendererUtil::createConstantBuffer(_lightBuffer, sizeof(PointLightType), device);
	initializeLight(device, "DeferredLight_vs.cso", "DeferredPointLight_ps.cso");
}

void PointLight::draw(const DrawSettings & settings, RefPtr<LightComponent>& lightComponent) {
	auto deviceContext = settings.deviceContext;

	ImGui::Begin("PointLight");
	static float X = 0.0;
	static float Y = 0.0;
	static float Z = 3.0;
	static float constantAttenuation = 0.0f;
	static float linearAttenuation = 0.0f;
	static float quadraticAttenuation = 0.03f;
	static Vector3 lightColor = Vector3::one;
	static float lightIntensity = 1.0f;

	ImGui::ColorEdit3("Color", &lightColor.x);
	ImGui::SliderFloat("X", &X, -30.0f, 30.0f);
	ImGui::SliderFloat("Y", &Y, -30.0f, 30.0f);
	ImGui::SliderFloat("Z", &Z, -30.0f, 30.0f);
	ImGui::NewLine();

	ImGui::SliderFloat("Constant", &constantAttenuation, 0.0f, 3.0f);
	ImGui::SliderFloat("Linear", &linearAttenuation, 0.0f, 3.0f);
	ImGui::SliderFloat("Quadratic", &quadraticAttenuation, 0.0f, 1.0f);
	ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 30.0f);
	ImGui::End();


	Matrix4 mtxProj = settings.camera->mtxProj;
	Matrix4 mtxView = settings.camera->mtxView;
	PointLightType lightBuffer;

	lightBuffer.position = Vector3(X, Y, Z);
	lightBuffer.color = lightColor * lightIntensity;
	lightBuffer.attenuation = Vector4(constantAttenuation, linearAttenuation, quadraticAttenuation, 0);
	lightBuffer.mtxViewProjInv = Matrix4::transpose(Matrix4::multiply(mtxProj.inverse(), mtxView));

	deviceContext->UpdateSubresource(_lightBuffer.Get(), 0, NULL, &lightBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _lightBuffer.GetAddressOf());
	deviceContext->PSSetShaderResources(4, 1, settings.deferredBuffers->getDepthStencilResource().GetAddressOf());

	Light::draw(settings, lightComponent);
}
