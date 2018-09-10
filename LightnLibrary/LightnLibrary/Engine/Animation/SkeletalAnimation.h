#pragma once

#include "AnimationBase.h"

class SkeletalAnimation :public AnimationBase {

public:

	SkeletalAnimation();

	~SkeletalAnimation();

	void load(const std::string& fileName) override;

	//アニメーションタイマ更新
	void updateTimer(float deltaTime, float overrideFrame = -1.0f);

	//アニメーション更新
	void computeBones(int32 rootMotionIndex = -1) override;

	//アニメーションをリセットする
	void resetAnimation() override;

	//再生スピードを指定
	void setPlayRate(float rate) override;

	//再生スピードを取得
	float getPlayRate() const override;

	//アニメーションの最大フレームを取得
	int32 getMaxFrame() const;

	float getPlaingFrame() const;
	float getBeforePlaingFrame() const;
	const std::vector<AnimationBone>& getAnimationBones() const;
	TransformQ getRootMotionTransform(uint32 rootMotionIndex) const override;

	//固有識別子を取得
	std::string getName() const override;

	//アニメーションのフレームキャッシュを取得
	const std::vector<TransformQ>& getFrameCache() const override;

private:

	Matrix4 getRootMotionMatrixInverse(uint32 rootMotionIndex) const;
	TransformQ getBoneTransformFromKey(uint32 boneIndex, uint32 keyIndex) const;
	void clampRootMotionTransform(TransformQ& transform) const;

private:

	std::vector<AnimationBone> _animationBones;
	std::vector<TransformQ> _frameCache;
	TransformQ _rootMotionTransform;
	TransformQ _rootMotionTransformSecond;
	std::string _name;

	uint32 _maxFrame;
	float _plaingFrame;
	float _beforePlaingFrame;
	float _playRate;
};