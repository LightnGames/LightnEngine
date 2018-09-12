#pragma once

#include <Component/LightComponent.h>
#include <Renderer/LightEntity.h>

class DirectionalLightComponent :public LightComponent, public LightEntity {

public:
	DirectionalLightComponent();
	~DirectionalLightComponent();

	void draw(const DrawSettings& drawSettings) override;
};

