#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <Util/RefPtr.h>

class SkeltalAnimation;
struct Avator;

using Animation = std::shared_ptr<SkeltalAnimation>;

class AnimationController{

	
public:

	AnimationController(RefPtr<Avator> skeleton);

	~AnimationController();

	//アニメーションリストに追加
	void addAnimationList(const Animation& animation);
	void addAnimationList(const std::string& name);

	//アニメーションを再生
	void play(const std::string& name, float blendTime = 0.6f);

	//アニメーションを更新
	void update(float deltaTime);

private:

	std::unordered_map<std::string, Animation> _playList;

	std::weak_ptr<SkeltalAnimation> _duringAnimation;
	std::weak_ptr<SkeltalAnimation> _blendingAnimation;

	float _blendingTime;
	float _blendTime;

	RefPtr<Avator> _avator;

};