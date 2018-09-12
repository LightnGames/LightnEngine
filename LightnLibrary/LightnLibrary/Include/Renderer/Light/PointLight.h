#pragma once

#include <Renderer/Light/Light.h>

struct PointLightType {
	Vector4 position;
	Vector4 color;
	Vector4 attenuation;
	Matrix4 mtxViewProjInv;
};

class PointLight :public Light {

public:

	PointLight();

	~PointLight();

	void initialize(ComPtr<ID3D11Device>& device);

	void draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent) override;

};