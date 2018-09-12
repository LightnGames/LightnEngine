#include "BlendSpace2D.h"
#include <cassert>
#include <algorithm>
#include "SkeletalAnimation.h"

void BlendSpace2D::load(const std::string & assetName) {
	_name = assetName;
}

void BlendSpace2D::addBlendAnimation(const std::string & name, float blendSpaceX, float blendSpaceY) {

	//すでにYと同じBlendSpace1Dがあればそこに追加
	for (auto&& b : _blends) {
		if (approximately(blendSpaceY, b.second)) {
			b.first.addBlendAnimation(name, blendSpaceX);
			return;
		}
	}

	//YのBlendSpaceが存在しないので新規追加
	BlendSpace1D addAnim;
	addAnim.load(name + std::to_string(_blends.size()));
	addAnim.addBlendAnimation(name, blendSpaceX);
	_blends.emplace_back(BlendSpace2DTask(addAnim, blendSpaceY));
}

void BlendSpace2D::setUpBlendSpace() {
	assert(!_blends.empty() && "ブレンドスペースのアニメーションが一つもありません");

	std::sort(
		_blends.begin(),
		_blends.end(),
		[](const BlendSpace2DTask& first, const BlendSpace2DTask& second) {return first.second < second.second; }
	);

	for (auto&& b : _blends) {
		b.first.setUpBlendSpace();
	}

	_frameCache = std::vector<TransformQ>(_blends[0].first.getFrameCache().size());
	_blendSpaceX = 0.0f;
	_blendSpaceY = 0.0f;
}

void BlendSpace2D::updateTimer(float deltaTime, float overrideFrame) {
	for (auto&& a : _blends) {
		a.first.updateTimer(deltaTime, overrideFrame);
	}
}

void BlendSpace2D::computeBones(int32 rootMotionIndex) {
	uint32 firstAnimationIndex = 0;
	uint32 secondAnimationIndex = 0;

	const uint32 animNum = _blends.size();
	_blendSpaceY = clamp(_blendSpaceY, _blends[0].second, _blends[animNum - 1].second);

	for (int i = 0; i < animNum; ++i) {
		if (_blends[i].second <= _blendSpaceY) {
			firstAnimationIndex = i;
		}

		const uint32 upIndex = animNum - i - 1;
		if (_blends[upIndex].second >= _blendSpaceY) {
			secondAnimationIndex = upIndex;
		}
	}

	BlendSpace2DTask& firstAnimTask = _blends[firstAnimationIndex];
	BlendSpace2DTask& secondAnimTask = _blends[secondAnimationIndex];
	const float blendSpaceRange = secondAnimTask.second - firstAnimTask.second;

	_firstBlendSpace = &firstAnimTask.first;
	_secondBlendSpace = &secondAnimTask.first;

	if (firstAnimationIndex == secondAnimationIndex) {
		_lerpValue = 0;
	}
	else {
		_lerpValue = (_blendSpaceY - firstAnimTask.second) / blendSpaceRange;
	}

	//ブレンドアニメーション更新
	_firstBlendSpace->setBlendSpace(_blendSpaceX);
	_firstBlendSpace->computeBones(rootMotionIndex);

	if (firstAnimationIndex != secondAnimationIndex) {
		_secondBlendSpace->setBlendSpace(_blendSpaceX);
		_secondBlendSpace->computeBones(rootMotionIndex);
	}

	const auto& firstFrameCaches = _firstBlendSpace->getFrameCache();
	const auto& secondFrameCaches = _secondBlendSpace->getFrameCache();
	for (int i = 0; i < firstFrameCaches.size(); ++i) {

		//参照するキーを取得
		const TransformQ& firstkey = firstFrameCaches[i];
		const TransformQ& secondKey = secondFrameCaches[i];

		//中間値で補完した各座標を取得
		const Vector3 lerpPosition = Vector3::lerp(firstkey.position, secondKey.position, _lerpValue);
		const Vector3 lerpScale = Vector3::lerp(firstkey.scale, secondKey.scale, _lerpValue);
		const Quaternion slerpRotation = Quaternion::slerp(firstkey.rotation, secondKey.rotation, _lerpValue);


		//補完したボーン情報をキャッシュする
		const TransformQ transformCache(lerpPosition, slerpRotation, lerpScale);
		_frameCache[i] = std::move(transformCache);
	}
}

void BlendSpace2D::resetAnimation() {
	for (auto&& a : _blends) {
		a.first.resetAnimation();
	}
}

void BlendSpace2D::setPlayRate(float rate) {
	for (auto&& a : _blends) {
		a.first.setPlayRate(rate);
	}
}

float BlendSpace2D::getPlayRate() const {
	return _blends[0].first.getPlayRate();
}

TransformQ BlendSpace2D::getRootMotionTransform(uint32 rootMotionIndex) const {
	const TransformQ& firstTransform = _firstBlendSpace->getRootMotionTransform(rootMotionIndex);
	const TransformQ& secondTransform = _secondBlendSpace->getRootMotionTransform(rootMotionIndex);

	TransformQ result;
	result.position = Vector3::lerp(firstTransform.position, secondTransform.position, _lerpValue);
	result.scale = Vector3::lerp(firstTransform.scale, secondTransform.scale, _lerpValue);
	result.rotation = Quaternion::slerp(firstTransform.rotation, secondTransform.rotation, _lerpValue);

	return result;
}

std::string BlendSpace2D::getName() const {
	return _name;
}

const std::vector<TransformQ>& BlendSpace2D::getFrameCache() const {
	return _frameCache;
}
