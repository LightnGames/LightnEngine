#pragma once

#include <Animation/AnimationBase.h>
#include <Util/Util.h>

class SkeletalAnimation;

using BlendSpace1DTask = std::pair<SkeletalAnimation, float>;

class BlendSpace1D :public AnimationBase {

public:
	BlendSpace1D();
	~BlendSpace1D();

	void load(const std::string& assetName) override;

	//ブレンドスペースにアニメーションを追加
	void addBlendAnimation(const std::string& name, float blendSpace);

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

	void setBlendSpace(float value) { _blendSpace = value; }
	float getBlendSpace() const { return _blendSpace; }


private:

	std::vector<BlendSpace1DTask> _anims;
	float _blendSpace;

	std::vector<TransformQ> _frameCache;
	RefPtr<SkeletalAnimation> _firstAnim;
	RefPtr<SkeletalAnimation> _secondAnim;
	float _lerpValue;

	float _playRate;
	std::string _name;

};