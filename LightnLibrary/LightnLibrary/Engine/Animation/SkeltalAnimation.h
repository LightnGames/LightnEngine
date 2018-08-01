#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <Transform.h>

struct Skeleton;

struct AnimationBone{

	std::vector<TransformQ> keys;
	Matrix4 animatedMatrix;

	TransformQ& operator[](int boneIndex){
		return keys[boneIndex];
	}
};

class SkeltalAnimation{

public:

	~SkeltalAnimation();

	SkeltalAnimation(const std::string& fileName);

	void load(const std::string& fileName);

	//アニメーション更新
	void update(float deltaTime);

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

	//固有識別子を取得
	std::string getName() const;

	//アニメーションのフレームキャッシュを取得
	const std::vector<TransformQ>& getFrameCache() const;
	
private:

	std::vector<AnimationBone> _animationBones;
	std::vector<TransformQ> _frameCache;
	std::string _name;

	int _maxFrame;
	float _plaingFrame;
	float _playRate;
};