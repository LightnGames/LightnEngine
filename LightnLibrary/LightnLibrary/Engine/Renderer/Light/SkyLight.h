#pragma once
#include "Light.h"

struct SkyLightType {
	Vector4 color;
	Vector4 intensity;
	Matrix4 mtxViewProjInv;
	Vector4 cameraPosition;
};

class SkyLight :public Light{
public:

	void initialize(ComPtr<ID3D11Device>& device);

	void draw(const DrawSettings& settings, RefPtr<LightComponent>& lightComponent) override;
};