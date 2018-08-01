#pragma once

#include <Actor/Actor.h>
#include <Components/DirectionalLightComponent.h>

class DirectionalLightActor :public Actor{

public:

	virtual void setUpTask() override;

	virtual void start() override;

	virtual void update(float deltaTime) override;

private:

	RefPtr<DirectionalLightComponent> _lightComponent;

};