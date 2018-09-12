#include <Actor/SkeletalMeshActor.h>

SkeletalMeshActor::SkeletalMeshActor() :_skeletalMeshComponent{ nullptr } {
}

void SkeletalMeshActor::start() {
}

void SkeletalMeshActor::setUpTask() {
	_skeletalMeshComponent = addComponent<SkeletalMeshComponent>();
	_animationComponent = addComponent<AnimationControllerComponent>();

}

void SkeletalMeshActor::setUpSkeletalMesh(const std::string & filePath, const std::vector<std::string>& matFiles)
{
	_skeletalMeshComponent->setUpSkeletalMesh(filePath, matFiles);
	_animationComponent->initialize(_skeletalMeshComponent->skeletalMesh()->avator());
}

void SkeletalMeshActor::update(float deltaTime) {
	Actor::update(deltaTime);
}
