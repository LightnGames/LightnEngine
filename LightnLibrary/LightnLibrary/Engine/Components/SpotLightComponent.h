#pragma once

#include <Components/LightComponent.h>
#include <Renderer/LightEntity.h>

class SpotLightComponent :public LightComponent, public LightEntity {

public:
	SpotLightComponent();
	~SpotLightComponent();

	void draw(const DrawSettings& drawSettings) override;
};