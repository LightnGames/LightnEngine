#pragma once

#include <Actor/Actor.h>
#include <Component/StaticInstanceMeshComponent.h>

class StaticInstanceMeshActor : public Actor {

public:

	StaticInstanceMeshActor();

	virtual void start() override;

	virtual void setUpTask() override;

	virtual void update(float deltaTime) override;

public:

	RefPtr<StaticInstanceMeshComponent> _staticInstanceMeshComponent;

};