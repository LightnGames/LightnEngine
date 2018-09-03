#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <Util/Type.h>
#include <Transform.h>

struct Skeleton;

struct AnimationBone{

	std::vector<TransformQ> keys;
	Matrix4 animatedMatrix;

	TransformQ& operator[](int boneIndex){
		return keys[boneIndex];
	}
};

#define LOOP_BLEND_RANGE 0.1f

class SkeltalAnimation{

public:

	~SkeltalAnimation();

	SkeltalAnimation(const std::string& fileName);

	void load(const std::string& fileName);

	//アニメーション更新
	void update(float deltaTime, int rootMotionIndex = -1, float overrideFrame = -1.0f);

	//アニメーションボーン変換行列を取得
	const Matrix4& getAnimationMatrix(int boneIndex) const;

	//アニメーションをリセットする
	void resetAnimation();

	//再生スピードを指定
	void setPlayRate(float rate);

	//再生スピードを取得
	float getPlayRate() const;

	//アニメーションの最大フレームを取得
	int getMaxFrame() const;

	float getPlaingFrame() const;
	float getBeforePlaingFrame() const;
	const std::vector<AnimationBone>& getAnimationBones() const;
	const TransformQ& getRootMotionTransform(bool second) const;

	//固有識別子を取得
	std::string getName() const;

	//アニメーションのフレームキャッシュを取得
	const std::vector<TransformQ>& getFrameCache() const;
	
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