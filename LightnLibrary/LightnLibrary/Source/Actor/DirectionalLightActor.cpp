#include <Actor/DirectionalLightActor.h>

void DirectionalLightActor::setUpTask() {
	Actor::setUpTask();
	_lightComponent = addComponent<DirectionalLightComponent>();
}

void DirectionalLightActor::start() {
	Actor::start();
}

void DirectionalLightActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
