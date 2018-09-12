#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <Util/Util.h>
#include <Transform.h>

struct Skeleton;

struct AnimationBone {

	std::vector<TransformQ> keys;

	TransformQ& operator[](int boneIndex) {
		return keys[boneIndex];
	}
};


class AnimationBase {

public:

	AnimationBase() {};

	virtual ~AnimationBase() {};

	virtual void load(const std::string& fileName) = 0;

	//アニメーションタイマ更新
	virtual void updateTimer(float deltaTime, float overrideFrame = -1.0f) = 0;

	//アニメーション更新
	virtual void computeBones(int32 rootMotionIndex = -1) = 0;

	//アニメーションをリセットする
	virtual void resetAnimation() = 0;

	//再生スピードを指定
	virtual void setPlayRate(float rate) = 0;

	//再生スピードを取得
	virtual float getPlayRate() const = 0;

	virtual TransformQ getRootMotionTransform(uint32 rootMotionIndex) const = 0;

	//固有識別子を取得
	virtual std::string getName() const = 0;

	//アニメーションのフレームキャッシュを取得
	virtual const std::vector<TransformQ>& getFrameCache() const = 0;

};