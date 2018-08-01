#include "StaticMeshActor.h"

StaticMeshActor::StaticMeshActor() :_staticMeshComponent{ nullptr } {
}

void StaticMeshActor::start() {
}

void StaticMeshActor::setUpTask() {
	Actor::setUpTask();
	_staticMeshComponent = addComponent<StaticMeshComponent>();
}

void StaticMeshActor::setUpStaticMesh(const std::string & filePath, const std::vector<std::string>& matFiles){
	_staticMeshComponent->setUpStaticMesh(filePath, matFiles);
}

void StaticMeshActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
