#include "SpotLightActor.h"

void SpotLightActor::setUpTask()
{
	Actor::setUpTask();
	_lightComponent = addComponent<SpotLightComponent>();
}

void SpotLightActor::start()
{
	Actor::start();
}

void SpotLightActor::update(float deltaTime)
{
	Actor::update(deltaTime);
}
