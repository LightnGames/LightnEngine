#include "PointLightComponent.h"
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Light/PointLight.h>

PointLightComponent::PointLightComponent()
{
	_light = GraphicsResourceManager::instance().getPointLight().cast<Light>();
	//SceneRendererManager::instance().addLightEntity(this);
	SceneRendererManager::instance().addPointLight(this);

	light.attenuationBegin = 1;
	light.attenuationEnd = 5;
	light.color = Vector3(1, 1, 1)*100;
}

PointLightComponent::~PointLightComponent()
{
	//SceneRendererManager::instance().removeLightEntity(this);
	SceneRendererManager::instance().removePointLight(this);
}

void PointLightComponent::draw(const DrawSettings & drawSettings)
{
	//_light->draw(drawSettings, RefPtr<LightComponent>(this));
}
