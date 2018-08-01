#pragma once

#include <Actor/Actor.h>
#include <Components/PointLightComponent.h>

class PointLightActor :public Actor {

public:

	virtual void setUpTask() override;

	virtual void start() override;

	virtual void update(float deltaTime) override;

private:

	RefPtr<PointLightComponent> _lightComponent;

};