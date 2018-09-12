#include <Component/DirectionalLightComponent.h>
#include <Renderer/SceneRendererManager.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Light/DirectionalLight.h>

DirectionalLightComponent::DirectionalLightComponent() {
	SceneRendererManager::instance().addLightEntity(this);
	_light = GraphicsResourceManager::instance().getDirectionalLight().cast<Light>();
	setShadowSize(2048);
	enableShadow(true);
}


DirectionalLightComponent::~DirectionalLightComponent() {
	SceneRendererManager::instance().removeLightEntity(this);
}

void DirectionalLightComponent::draw(const DrawSettings& drawSettings) {
	_light->draw(drawSettings, RefPtr<LightComponent>(this));
}
