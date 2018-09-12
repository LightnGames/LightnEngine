#include <Animation/BlendSpace1D.h>
#include <Animation/SkeletalAnimation.h>
#include <algorithm>
#include <cassert>
#include <ThirdParty/ImGui/imgui.h>

BlendSpace1D::BlendSpace1D() :_blendSpace{ 0.0f }{
}

BlendSpace1D::~BlendSpace1D(){
}

void BlendSpace1D::load(const std::string & assetName){
	_name = assetName;
}

void BlendSpace1D::addBlendAnimation(const std::string & name, float blendSpace){

	SkeletalAnimation addAnim;
	addAnim.load(name);

	_anims.emplace_back(BlendSpace1DTask(addAnim, blendSpace));
}

void BlendSpace1D::setUpBlendSpace(){

	assert(!_anims.empty() && "ブレンドスペースのアニメーションが一つもありません");

	std::sort(
		_anims.begin(),
		_anims.end(),
		[](const BlendSpace1DTask& first, const BlendSpace1DTask& second){return first.second < second.second; }
	);

	_frameCache = std::vector<TransformQ>(_anims[0].first.getAnimationBones().size());
	_blendSpace = 0.0f;
}

void BlendSpace1D::updateTimer(float deltaTime, float overrideFrame){
	for (auto&& a : _anims){
		a.first.updateTimer(deltaTime, overrideFrame);
	}
}

void BlendSpace1D::computeBones(int32 rootMotionIndex){

	uint32 firstAnimationIndex = 0;
	uint32 secondAnimationIndex = 0;

	const auto animNum = _anims.size();
	_blendSpace = clamp(_blendSpace, _anims[0].second, _anims[animNum - 1].second);

	for (auto i = 0; i < animNum; ++i){
		if (_anims[i].second <= _blendSpace){
			firstAnimationIndex = i;
		}

		const auto upIndex = animNum - i - 1;
		if (_anims[upIndex].second >= _blendSpace) {
			secondAnimationIndex = static_cast<uint32>(upIndex);
		}
	}

	BlendSpace1DTask& firstAnimTask = _anims[firstAnimationIndex];
	BlendSpace1DTask& secondAnimTask = _anims[secondAnimationIndex];
	const float blendSpaceRange = secondAnimTask.second - firstAnimTask.second;

	_firstAnim = &firstAnimTask.first;
	_secondAnim = &secondAnimTask.first;

	if (firstAnimationIndex == secondAnimationIndex){
		_lerpValue = 0;
	}
	else{
		_lerpValue = (_blendSpace - firstAnimTask.second) / blendSpaceRange;
	}

	//ブレンドアニメーション更新
	_firstAnim->computeBones(rootMotionIndex);
	if ((firstAnimationIndex != secondAnimationIndex)) {
		_secondAnim->computeBones(rootMotionIndex);
	}


	const auto& firstFrameCaches = _firstAnim->getFrameCache();
	const auto& secondFrameCaches = _secondAnim->getFrameCache();
	for (int i = 0; i < firstFrameCaches.size(); ++i){

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

void BlendSpace1D::resetAnimation(){
	for (auto&& a : _anims){
		a.first.resetAnimation();
	}
}

void BlendSpace1D::setPlayRate(float rate){
	for (auto&& a : _anims){
		a.first.setPlayRate(rate);
	}
}

float BlendSpace1D::getPlayRate() const{
	return _anims[0].first.getPlayRate();
}

TransformQ BlendSpace1D::getRootMotionTransform(uint32 rootMotionIndex) const{
	const TransformQ& firstTransform = _firstAnim->getRootMotionTransform(rootMotionIndex);
	const TransformQ& secondTransform = _secondAnim->getRootMotionTransform(rootMotionIndex);

	TransformQ result;
	result.position = Vector3::lerp(firstTransform.position, secondTransform.position, _lerpValue);
	result.scale = Vector3::lerp(firstTransform.scale, secondTransform.scale, _lerpValue);
	result.rotation = Quaternion::slerp(firstTransform.rotation, secondTransform.rotation, _lerpValue);

	return result;
}

std::string BlendSpace1D::getName() const{
	return _name;
}

const std::vector<TransformQ>& BlendSpace1D::getFrameCache() const{
	return _frameCache;
}