#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <Util/RefPtr.h>
#include <LMath.h>
#include <list>

class SkeltalAnimation;
struct Avator;

using Animation = std::unique_ptr<SkeltalAnimation>;

struct AnimTask {
	RefPtr<SkeltalAnimation> anim;
	float blendingTime;
	float blendTime;
	float blendFactor;
};

class AnimationController{

	
public:

	AnimationController(RefPtr<Avator> skeleton);

	~AnimationController();

	//アニメーションリストに追加
	void addAnimationList(Animation animation);
	void addAnimationList(const std::string& name);

	//アニメーションを再生
	void play(const std::string& name, float blendTime = 0.6f);

	//アニメーションを更新
	void update(float deltaTime);

	float debugTime = -1.0f;
	bool applyRootMotion = false;
	int rootMotionIndex = 0;
	TransformQ rootMotionVelocity;

	void setRootMotionBone(const std::string& boneName);

private:

	std::unordered_map<std::string, Animation> _playList;

	std::list<AnimTask> _duringAnimations;
	RefPtr<SkeltalAnimation> _duringAnimation;
	RefPtr<SkeltalAnimation> _blendingAnimation;

	float _blendingTime;
	float _blendTime;

	RefPtr<Avator> _avator;

};