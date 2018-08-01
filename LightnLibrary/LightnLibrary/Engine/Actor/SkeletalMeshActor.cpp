#include "SkeletalMeshActor.h"

SkeletalMeshActor::SkeletalMeshActor() :_skeletalMeshComponent{ nullptr } {
}

void SkeletalMeshActor::start() {
}

void SkeletalMeshActor::setUpTask() {
	_skeletalMeshComponent = addComponent<SkeletalMeshComponent>();
	_animationComponent = addComponent<AnimationControllerComponent>();
	_camera = addComponent<CameraComponent>();
	CameraComponent::mainCamera = _camera;

	_camera->setWorldPosition({0.7f, 1.6f, -2.3f});
	_camera->setWorldRotation(Quaternion::euler({ 0, 0, 0 }));
}

void SkeletalMeshActor::setUpSkeletalMesh(const std::string & filePath, const std::vector<std::string>& matFiles)
{
	_skeletalMeshComponent->setUpSkeletalMesh(filePath, matFiles);
	_animationComponent->initialize(_skeletalMeshComponent->skeletalMesh()->avator());
}

void SkeletalMeshActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
