#include "SkyLightComponent.h"
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Light/DirectionalLight.h>

SkyLightComponent::SkyLightComponent()
{
	_light = GraphicsResourceManager::instance().getSkyLight().cast<Light>();
	SceneRendererManager::instance().addLightEntity(this);
}

SkyLightComponent::~SkyLightComponent()
{
	SceneRendererManager::instance().removeLightEntity(this);
}

void SkyLightComponent::draw(const DrawSettings & drawSettings)
{
	_light->draw(drawSettings, RefPtr<LightComponent>(this));
}
