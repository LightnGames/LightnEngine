#include "SkyboxActor.h"
#include <Renderer/RendererSettings.h>

SkyboxActor::SkyboxActor() {
}

void SkyboxActor::start() {
	Actor::start();
}

void SkyboxActor::setUpTask() {
	Actor::setUpTask();
	_skyLightComponent = addComponent<SkyLightComponent>();
}

void SkyboxActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
