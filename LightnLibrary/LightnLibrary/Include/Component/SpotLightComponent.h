#pragma once

#include <Component/LightComponent.h>
#include <Renderer/RenderableInterface.h>
#include <Renderer/Light/LightTypes.h>

class SpotLightComponent :public LightComponent, public LightEntity {

public:
	SpotLightComponent();
	~SpotLightComponent();

	void draw(const DrawSettings& drawSettings) override;

	TileBasedSpotLightType light;
};