#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <Util/RefPtr.h>
#include <LMath.h>
#include <list>

class AnimationBase;
struct Avator;

struct AnimTask {
	RefPtr<AnimationBase> anim;
	float blendingTime;
	float blendTime;
	float blendFactor;
};

class AnimationController{

	
public:

	AnimationController(RefPtr<Avator> skeleton);

	~AnimationController();

	//アニメーションリストに追加
	void addAnimationList(std::unique_ptr<AnimationBase> animation);

	template<class T>
	RefPtr<T> addAnimationList(const std::string& name) {
		auto anim = new T();
		anim->load(name);
		addAnimationList(std::move(std::unique_ptr<T>(anim)));

		return anim;
	}

	//アニメーションを再生
	void play(const std::string& name, float blendTime = 0.6f);

	//アニメーションを更新
	void update(float deltaTime);

	float debugTime = -1.0f;
	bool applyRootMotion = false;
	int rootMotionIndex = 0;
	TransformQ rootMotionVelocity;

	void setRootMotionBone(const std::string& boneName);

	template<class T>
	RefPtr<T> getAnimation(const std::string& name) {
		return static_cast<T*>(_playList[name].get());
	}

private:

	std::unordered_map<std::string, std::unique_ptr<AnimationBase>> _playList;

	std::list<AnimTask> _duringAnimations;
	RefPtr<AnimationBase> _duringAnimation;
	RefPtr<AnimationBase> _blendingAnimation;

	float _blendingTime;
	float _blendTime;

	RefPtr<Avator> _avator;

};