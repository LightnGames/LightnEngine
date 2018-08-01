#include "PointLightComponent.h"
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Light/PointLight.h>

PointLightComponent::PointLightComponent()
{
	_light = GraphicsResourceManager::instance().getPointLight().cast<Light>();
	SceneRendererManager::instance().addLightEntity(this);
}

PointLightComponent::~PointLightComponent()
{
	SceneRendererManager::instance().removeLightEntity(this);
}

void PointLightComponent::draw(const DrawSettings & drawSettings)
{
	_light->draw(drawSettings, RefPtr<LightComponent>(this));
}
