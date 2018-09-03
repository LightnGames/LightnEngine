#include "AnimationControllerComponenth.h"
#include "Animation/AnimationManager.h"

AnimationControllerComponent::AnimationControllerComponent() {
}

void AnimationControllerComponent::initialize(RefPtr<Avator> avator) {
	_animationController = std::make_unique<AnimationController>(avator);

	//_animationController->addAnimationList("ZombieRun.anim");
	//_animationController->play("ZombieRun");
}

void AnimationControllerComponent::update(float deltaTime) {
	Component::update(deltaTime);
	_animationController->update(deltaTime);
}

void AnimationControllerComponent::play(const std::string & name, float blendTime)
{
	_animationController->play(name, blendTime);
}
