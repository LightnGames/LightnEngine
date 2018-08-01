#pragma once

#include "Actor.h"
#include <Components/StaticMeshComponent.h>

class StaticMeshActor : public Actor {

public:

	StaticMeshActor();

	virtual void start() override;

	virtual void setUpTask() override;

	//スタティックメッシュをセットアップ
	virtual void setUpStaticMesh(const std::string& filePath, const std::vector<std::string>& matFiles);

	virtual void update(float deltaTime) override;

protected:

	RefPtr<StaticMeshComponent> _staticMeshComponent;

};