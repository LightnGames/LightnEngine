#pragma once

#include "Actor.h"
#include <Components/StaticMeshComponent.h>
#include <Components/SkyLightComponent.h>

class SkyboxActor :public Actor{

public:

	SkyboxActor();

	virtual void start() override;
	virtual void setUpTask() override;
	virtual void update(float deltaTime) override;

	//スカイボックスメッシュをセットアップ
	void setUpStaticMesh(const std::string& filePath, const std::vector<std::string>& matFiles);

protected:

	RefPtr<StaticMeshComponent> _staticMeshComponent;
	RefPtr<SkyLightComponent> _skyLightComponent;

};