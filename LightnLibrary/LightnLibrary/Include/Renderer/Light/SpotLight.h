#pragma once

#include <Renderer/Light/Light.h>

struct SpotLightType {
	Vector4 position;
	Vector4 direction;
	Vector4 color;
	Vector4 attenuation;
	Matrix4 mtxViewProjInv;
};

class SpotLight :public Light {

public:

	SpotLight();

	~SpotLight();

	void initialize(ComPtr<ID3D11Device>& device);

	void draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent) override;

};