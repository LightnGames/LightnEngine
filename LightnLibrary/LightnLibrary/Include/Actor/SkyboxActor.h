#pragma once

#include <Actor/Actor.h>
#include <Component/StaticMeshComponent.h>
#include <Component/SkyLightComponent.h>

class SkyboxActor :public Actor{

public:

	SkyboxActor();

	virtual void start() override;
	virtual void setUpTask() override;
	virtual void update(float deltaTime) override;

protected:

	RefPtr<SkyLightComponent> _skyLightComponent;

};