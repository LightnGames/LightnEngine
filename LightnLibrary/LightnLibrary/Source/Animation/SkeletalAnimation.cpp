#include <Animation/SkeletalAnimation.h>
#include <IO/MeshLoader.h>
#include <Renderer/Mesh/Skeleton.h>
#include <fstream>
#include <algorithm>
#include <iostream>

SkeletalAnimation::SkeletalAnimation() :
	_maxFrame{ 0 }, _playRate{ 1.0f }, _plaingFrame{ 0.0f }, _beforePlaingFrame{ 0.0f } {
}

SkeletalAnimation::~SkeletalAnimation() {
}

void SkeletalAnimation::load(const std::string & fileName) {

	//ファイル名から拡張子を除いて固有識別子にする
	uint32 path_i = static_cast<uint32>(fileName.find_last_of("/") + 1);
	_name = fileName.substr(path_i);

	path_i = static_cast<uint32>(_name.find_last_of("."));
	_name = _name.substr(0, path_i);

	//  ios::in は読み込み専用  ios::binary はバイナリ形式
	std::ifstream fin(fileName, std::ios::in | std::ios::binary);

	if (!fin) {
		std::cout << "ファイル " << fileName << " が開けません";
	}

	//ボーンサイズ
	uint32 boneSize = 0;
	fin.read(reinterpret_cast<char*>(&boneSize), sizeof(int));

	//キーサイズ
	uint32 keySize = 0;
	fin.read(reinterpret_cast<char*>(&keySize), sizeof(int));

	//最大フレーム数
	fin.read(reinterpret_cast<char*>(&_maxFrame), sizeof(int));

	//ボーン数だけメモリを確保
	_animationBones.reserve(boneSize);
	_frameCache.reserve(boneSize);
	_frameCache = std::vector<TransformQ>(boneSize);

	//ボーンすべて読み終わるまでループ
	while (_animationBones.size() != boneSize) {

		AnimationBone bone;

		//ボーン内のキーフレームを読み終わるまでループ
		while (bone.keys.size() != keySize) {

			//キーフレーム構造体をバイナリデータから生成
			TransformQ key;

			fin.read(reinterpret_cast<char*>(&key.position), sizeof(Vector3));
			fin.read(reinterpret_cast<char*>(&key.rotation), sizeof(Quaternion));
			fin.read(reinterpret_cast<char*>(&key.scale), sizeof(Vector3));

			bone.keys.emplace_back(key);
		}

		_animationBones.emplace_back(bone);
	}

	fin.close();
}

void SkeletalAnimation::updateTimer(float deltaTime, float overrideFrame) {

	_rootMotionTransformSecond = _rootMotionTransform;

	//フレーム更新
	//逆再生には未対応
	_beforePlaingFrame = _plaingFrame;
	_plaingFrame += 0.5f * getPlayRate();

	if (overrideFrame >= 0.0f) {
		_plaingFrame = overrideFrame;
	}

	//配列インデックスのため最大数-1でループ
	if (_plaingFrame >= _maxFrame - 1) {
		_plaingFrame -= _maxFrame - 1;
	}
}

void SkeletalAnimation::computeBones(int32 rootMotionIndex) {

	//どのフレーム間か調べる
	const uint32 firstFrame = static_cast<uint32>(std::floorf(_plaingFrame));
	const uint32 secondFrame = static_cast<uint32>(std::ceilf(_plaingFrame));

	//フレームの中間値(0.0f~1.0f)
	const float lerpValue = _plaingFrame - firstFrame;

	for (int i = 0; i < _animationBones.size(); i++) {

		//参照するキーを取得
		const TransformQ& firstkey = _animationBones[i][firstFrame];
		const TransformQ& secondKey = _animationBones[i][secondFrame];

		//中間値で補完した各座標を取得
		const Vector3 lerpPosition = Vector3::lerp(firstkey.position, secondKey.position, lerpValue);
		const Vector3 lerpScale = Vector3::lerp(firstkey.scale, secondKey.scale, lerpValue);
		const Quaternion slerpRotation = Quaternion::slerp(firstkey.rotation, secondKey.rotation, lerpValue);

		if (i == rootMotionIndex) {
			_rootMotionTransform.position = lerpPosition;
			_rootMotionTransform.scale = lerpScale;
			_rootMotionTransform.rotation = slerpRotation;
		}

		//ループアニメーションの繋ぎ補完
		/*if (blendFrame > 0) {
			const TransformQ& startTrans = _animationBones[i][0];
			const float blendAlpha = blendFrame / static_cast<float>(loopBlend + 1);

			lerpPosition = Vector3::lerp(lerpPosition, startTrans.position, blendAlpha);
			lerpScale = Vector3::lerp(lerpScale, startTrans.scale, blendAlpha);
			slerpRotation = Quaternion::slerp(slerpRotation, startTrans.rotation, blendAlpha);
		}*/

		//補完したボーン情報をキャッシュする
		const TransformQ transformCache(lerpPosition, slerpRotation, lerpScale);
		_frameCache[i] = std::move(transformCache);
	}

	if (rootMotionIndex == -1) {
		return;
	}

	Matrix4 mtxRootMotionBlend = getRootMotionMatrixInverse(rootMotionIndex);
	for (auto&& c : _frameCache) {
		c.position = Matrix4::transform(c.position, mtxRootMotionBlend);
		c.rotation = mtxRootMotionBlend.rotation()*c.rotation;
	}
}

void SkeletalAnimation::resetAnimation() {
	_plaingFrame = 0;
}

void SkeletalAnimation::setPlayRate(float rate) {
	_playRate = rate;
}

float SkeletalAnimation::getPlayRate() const {
	return _playRate;
}

int32 SkeletalAnimation::getMaxFrame() const {
	return _maxFrame;
}

float SkeletalAnimation::getPlaingFrame() const {
	return _plaingFrame;
}

float SkeletalAnimation::getBeforePlaingFrame() const {
	return _beforePlaingFrame;
}

const std::vector<AnimationBone>& SkeletalAnimation::getAnimationBones() const {
	return _animationBones;
}

TransformQ SkeletalAnimation::getRootMotionTransform(uint32 rootMotionIndex) const {

	Quaternion addq = Quaternion::identity;
	Vector3 addp = Vector3::zero;
	if (getPlaingFrame() - getBeforePlaingFrame() <= 0) {
		const int32 maxKeyIndex = getMaxFrame() - 1;

		const TransformQ& maxKeyTransform = getBoneTransformFromKey(rootMotionIndex, maxKeyIndex);
		const TransformQ& firstKeyTransform = getBoneTransformFromKey(rootMotionIndex, 0);

		addp = maxKeyTransform.position - firstKeyTransform.position;
		addq = maxKeyTransform.rotation*firstKeyTransform.rotation.inverse();
	}

	TransformQ result;
	result.position = _rootMotionTransform.position - _rootMotionTransformSecond.position + addp;
	result.scale = _rootMotionTransform.scale - _rootMotionTransformSecond.scale;
	result.rotation = addq * _rootMotionTransform.rotation*_rootMotionTransformSecond.rotation.inverse();
	
	clampRootMotionTransform(result);

	return std::move(result);
}

std::string SkeletalAnimation::getName() const {
	return _name;
}

const std::vector<TransformQ>& SkeletalAnimation::getFrameCache() const {
	return _frameCache;
}

Matrix4 SkeletalAnimation::getRootMotionMatrixInverse(uint32 rootMotionIndex) const {
	TransformQ rootMotionTransform = _frameCache[rootMotionIndex];
	clampRootMotionTransform(rootMotionTransform);
	Matrix4 mtxRootMotionBlend = Matrix4::createWorldMatrix(rootMotionTransform.position,
		rootMotionTransform.rotation, Vector3::one).inverse();

	return mtxRootMotionBlend;
}

TransformQ SkeletalAnimation::getBoneTransformFromKey(uint32 boneIndex, uint32 keyIndex) const {
	return getAnimationBones()[boneIndex].keys[keyIndex];
}

void SkeletalAnimation::clampRootMotionTransform(TransformQ & transform) const {
	transform.position.y = 0;
	transform.rotation = Quaternion::euler({ 0, transform.rotation.getYaw(), 0 }, true);
}
