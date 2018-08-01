#include "AnimationManager.h"

template<> AnimationManager* Singleton<AnimationManager>::mSingleton = 0;

AnimationManager::AnimationManager() {
}

void AnimationManager::updateAnimations(float deltaTime) {
}
