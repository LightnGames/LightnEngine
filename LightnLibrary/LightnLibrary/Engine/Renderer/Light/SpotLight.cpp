#include "SpotLight.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <ThirdParty/ImGui/imgui.h>
#include <Components/LightComponent.h>
#include <Components/CameraComponent.h>


SpotLight::SpotLight()
{
}

SpotLight::~SpotLight()
{
}

void SpotLight::initialize(ComPtr<ID3D11Device>& device)
{
	RendererUtil::createConstantBuffer(_lightBuffer, sizeof(SpotLightType), device);
	initializeLight(device, "DeferredLight_vs.cso", "DeferredSpotLight_ps.cso");
}

void SpotLight::draw(const DrawSettings & settings, RefPtr<LightComponent>& lightComponent)
{
	auto deviceContext = settings.deviceContext;

	ImGui::Begin("SpotLight");
	static float X = 4.46;
	static float Y = 1.55;
	static float Z = 3.0;

	static float pitchDir = 90.0;
	static float yawDir = 0.0;
	static float lightPow = 2.0f;

	static float constantAttenuation = 0.5f;
	static float linearAttenuation = 0.6f;
	static float quadraticAttenuation = 0.02f;
	static Vector3 lightColor = Vector3::one;
	static float lightIntensity = 1.0f;

	ImGui::ColorEdit3("Color", &lightColor.x);
	ImGui::SliderFloat("X", &X, -30.0f, 30.0f);
	ImGui::SliderFloat("Y", &Y, -30.0f, 30.0f);
	ImGui::SliderFloat("Z", &Z, -30.0f, 30.0f);
	ImGui::NewLine();

	ImGui::SliderFloat("Pitch", &pitchDir, 0.0f, 360.0f);
	ImGui::SliderFloat("Yaw", &yawDir, 0.0f, 360.0f);
	ImGui::SliderFloat("Pow", &lightPow, 0.0f, 180.0f);

	ImGui::NewLine();

	ImGui::SliderFloat("Constant", &constantAttenuation, 0.0f, 3.0f);
	ImGui::SliderFloat("Linear", &linearAttenuation, 0.0f, 3.0f);
	ImGui::SliderFloat("Quadratic", &quadraticAttenuation, 0.0f, 1.0f);
	ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 30.0f);
	ImGui::End();


	Matrix4 mtxProj = CameraComponent::mainCamera->mtxProj();
	Matrix4 mtxView = CameraComponent::mainCamera->cameraMatrix();
	SpotLightType lightBuffer;

	lightBuffer.position = Vector3(X, Y, Z);
	lightBuffer.color = lightColor * lightIntensity;
	lightBuffer.attenuation = Vector4(constantAttenuation, linearAttenuation, quadraticAttenuation, 0);
	lightBuffer.mtxViewProjInv = Matrix4::transpose(Matrix4::multiply(mtxProj.inverse(), mtxView));

	Vector3 lightDirection = Vector3::forward;
	lightDirection = Quaternion::rotVector(Quaternion::euler({ pitchDir,yawDir,0 }), lightDirection);
	lightBuffer.direction = lightDirection;
	lightBuffer.direction.w = lightPow;

	deviceContext->UpdateSubresource(_lightBuffer.Get(), 0, NULL, &lightBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, _lightBuffer.GetAddressOf());
	deviceContext->PSSetShaderResources(4, 1, settings.deferredBuffers->getDepthStencilResource().GetAddressOf());

	Light::draw(settings, lightComponent);
}
