#pragma once
#include "Light.h"

struct DirectionalLightType {
	Vector4 direction;
	Vector4 lightColor;
};

class DirectionalLight :public Light{

public:

	DirectionalLight();
	~DirectionalLight();

	void initialize(ComPtr<ID3D11Device>& device);

	void draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent) override;
};