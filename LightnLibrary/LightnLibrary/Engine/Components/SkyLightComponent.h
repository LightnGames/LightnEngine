#pragma once

#include <Components/LightComponent.h>
#include <Renderer/LightEntity.h>

class SkyLightComponent :public LightComponent, public LightEntity {

public:
	SkyLightComponent();
	~SkyLightComponent();

	void draw(const DrawSettings& drawSettings) override;
};

