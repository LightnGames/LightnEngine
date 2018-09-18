#pragma once

#include <Component/LightComponent.h>
#include <Renderer/RenderableInterface.h>
#include <Renderer/Light/LightTypes.h>

class PointLightComponent :public LightComponent, public LightEntity {

public:
	PointLightComponent();
	~PointLightComponent();

	void draw(const DrawSettings& drawSettings) override;

	TileBasedPointLightType light;
};