#pragma once

#include <Actor/Actor.h>
#include <Component/PointLightComponent.h>

class PointLightActor :public Actor {

public:

	virtual void setUpTask() override;

	virtual void start() override;

	virtual void update(float deltaTime) override;

public:

	RefPtr<PointLightComponent> _lightComponent;

};