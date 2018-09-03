#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <Util/RefPtr.h>
#include <LMath.h>

class SkeltalAnimation;
struct Avator;

using Animation = std::shared_ptr<SkeltalAnimation>;

class AnimationController{

	
public:

	AnimationController(RefPtr<Avator> skeleton);

	~AnimationController();

	//�A�j���[�V�������X�g�ɒǉ�
	void addAnimationList(const Animation& animation);
	void addAnimationList(const std::string& name);

	//�A�j���[�V�������Đ�
	void play(const std::string& name, float blendTime = 0.6f);

	//�A�j���[�V�������X�V
	void update(float deltaTime);

	float debugTime = -1.0f;
	bool applyRootMotion = false;
	int rootMotionIndex = 0;
	TransformQ rootMotionVelocity;

	void setRootMotionBone(const std::string& boneName);

private:

	std::unordered_map<std::string, Animation> _playList;

	std::weak_ptr<SkeltalAnimation> _duringAnimation;
	std::weak_ptr<SkeltalAnimation> _blendingAnimation;

	float _blendingTime;
	float _blendTime;

	RefPtr<Avator> _avator;

};