#pragma once

#include <Component/Component.h>
#include <Animation/AnimationController.h>
#include <Renderer/Mesh/Skeleton.h>

class AnimationControllerComponent : public Component {

public:
	AnimationControllerComponent();

	void initialize(RefPtr<Avator> avator);

	virtual void update(float deltaTime) override;

	void play(const std::string & name, float blendTime = 0.6f);

public:

	std::unique_ptr<AnimationController> _animationController;

};