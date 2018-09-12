#pragma once

#include "BlendSpace1D.h"

using BlendSpace2DTask = std::pair<BlendSpace1D, float>;

class BlendSpace2D :public AnimationBase {

public:

	void load(const std::string& assetName) override;

	//ブレンドスペースにアニメーションを追加
	void addBlendAnimation(const std::string& name, float blendSpaceX, float blendSpaceY);

	//ブレンドスペースアニメーションを構築
	void setUpBlendSpace();

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

	TransformQ getRootMotionTransform(uint32 rootMotionIndex) const override;

	//固有識別子を取得
	std::string getName() const override;

	//アニメーションのフレームキャッシュを取得
	const std::vector<TransformQ>& getFrameCache() const override;

	void setBlendSpace(float valueX, float valueY) { _blendSpaceX = valueX; _blendSpaceY = valueY; }
	Vector2 getBlendSpace() const { return Vector2(_blendSpaceX, _blendSpaceY); }

private:

	std::vector<BlendSpace2DTask> _blends;
	float _blendSpaceX;
	float _blendSpaceY;

	std::vector<TransformQ> _frameCache;
	RefPtr<BlendSpace1D> _firstBlendSpace;
	RefPtr<BlendSpace1D> _secondBlendSpace;
	float _lerpValue;

	float _playRate;
	std::string _name;
};