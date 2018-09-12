#include <Actor/PointLightActor.h>

void PointLightActor::setUpTask()
{
	Actor::setUpTask();
	_lightComponent = addComponent<PointLightComponent>();
}

void PointLightActor::start()
{
	Actor::start();
}

void PointLightActor::update(float deltaTime)
{
	Actor::update(deltaTime);
}
