#include "SpotLightComponent.h"
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Light/SpotLight.h>

SpotLightComponent::SpotLightComponent()
{
	_light = GraphicsResourceManager::instance().getSpotLight().cast<Light>();
	SceneRendererManager::instance().addLightEntity(this);
}

SpotLightComponent::~SpotLightComponent()
{
	SceneRendererManager::instance().removeLightEntity(this);
}

void SpotLightComponent::draw(const DrawSettings & drawSettings)
{
	_light->draw(drawSettings, RefPtr<LightComponent>(this));
}
