#include <Actor/StaticInstanceMeshActor.h>

StaticInstanceMeshActor::StaticInstanceMeshActor() :_staticInstanceMeshComponent{ nullptr } {
}

void StaticInstanceMeshActor::start() {
	Actor::start();
}

void StaticInstanceMeshActor::setUpTask() {
	Actor::setUpTask();
	_staticInstanceMeshComponent = addComponent<StaticInstanceMeshComponent>();
}

void StaticInstanceMeshActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
