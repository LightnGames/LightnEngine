#pragma once

#include <Actor/Actor.h>
#include <Components/SpotLightComponent.h>

class SpotLightActor :public Actor {

public:

	virtual void setUpTask() override;

	virtual void start() override;

	virtual void update(float deltaTime) override;

public:

	RefPtr<SpotLightComponent> _lightComponent;

};