#include "SpotLightComponent.h"
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Light/SpotLight.h>

SpotLightComponent::SpotLightComponent()
{
	_light = GraphicsResourceManager::instance().getSpotLight().cast<Light>();
	//SceneRendererManager::instance().addLightEntity(this);
	SceneRendererManager::instance().addSpotLight(this);

	light.attenuationBegin = 1;
	light.attenuationEnd = 10;
	light.color = Vector3(1, 1, 1)*10;
}

SpotLightComponent::~SpotLightComponent()
{
	//SceneRendererManager::instance().removeLightEntity(this);
	SceneRendererManager::instance().removeSpotLight(this);
}

void SpotLightComponent::draw(const DrawSettings & drawSettings)
{
	//_light->draw(drawSettings, RefPtr<LightComponent>(this));
}
