#include "SkyboxActor.h"
#include <Renderer/RendererSettings.h>

SkyboxActor::SkyboxActor() {
}

void SkyboxActor::start() {
	Actor::start();
}

void SkyboxActor::setUpTask() {
	Actor::setUpTask();
	_staticMeshComponent = addComponent<StaticMeshComponent>();
	_skyLightComponent = addComponent<SkyLightComponent>();
}

void SkyboxActor::setUpStaticMesh(const std::string & filePath, const std::vector<std::string>& matFiles){
	_staticMeshComponent->setUpStaticMesh(filePath, matFiles);
	RendererSettings::skyBox = _staticMeshComponent->staticMesh()->material(0)->ppTextures[0];
}

void SkyboxActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
