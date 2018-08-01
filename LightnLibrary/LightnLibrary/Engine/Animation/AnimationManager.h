#pragma once

#include "../Util/Singleton.h"

class AnimationManager :public Singleton<AnimationManager> {

public:

	AnimationManager();

	void updateAnimations(float deltaTime);

};