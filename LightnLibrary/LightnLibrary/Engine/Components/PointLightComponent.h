#pragma once

#include <Components/LightComponent.h>
#include <Renderer/LightEntity.h>

class PointLightComponent :public LightComponent, public LightEntity {

public:
	PointLightComponent();
	~PointLightComponent();

	void draw(const DrawSettings& drawSettings) override;
};