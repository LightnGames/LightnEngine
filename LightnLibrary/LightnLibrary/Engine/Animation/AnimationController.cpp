#include "AnimationController.h"
#include "SkeltalAnimation.h"
#include "../Renderer/Mesh/Skeleton.h"
#include <ThirdParty/ImGui/imgui.h>

AnimationController::AnimationController(RefPtr<Avator> avator) :_avator{ avator }, _blendingTime{ 0.0f }, _blendTime{ 0.0f } {
}

AnimationController::~AnimationController() {
}

void AnimationController::addAnimationList(const Animation & animation) {
	_playList[animation->getName()] = animation;
}

void AnimationController::addAnimationList(const std::string & name) {
	auto anim = std::make_shared<SkeltalAnimation>(name);
	addAnimationList(anim);
}

void AnimationController::play(const std::string & name, float blendTime) {

	if (_playList.count(name) == 0) {
		return;
	}

	if (_duringAnimations.empty()) {
		blendTime = 0.0f;
	}

	AnimTask task;
	task.anim = _playList[name];
	task.blendingTime = blendTime;
	task.blendTime = blendTime;
	_duringAnimations.emplace_back(std::move(task));

	_blendingTime = blendTime;
	_blendTime = blendTime;
}

void AnimationController::update(float deltaTime) {

	rootMotionVelocity.position = Vector3::zero;
	rootMotionVelocity.rotation = Quaternion::identity;
	rootMotionVelocity.scale = Vector3::one;

	//アニメーションを全部計算してから再度取得しているのでメモリ効率極めて悪し
	//anim->update(deltaTime, rootMotionIndex, debugTime);
	for (auto itr = _duringAnimations.begin(); itr != _duringAnimations.end(); ++itr) {

		auto anim = itr->anim;

		//各種アニメーション時間更新
		anim->update(deltaTime, rootMotionIndex, debugTime);
		itr->blendingTime -= 0.016666f;
		
		//不要になったアニメーションを破棄
		if (itr->blendingTime <= 0) {
			itr->blendingTime = 0.0f;

			if (itr != _duringAnimations.begin()) {
				itr = _duringAnimations.erase(--itr);
				continue;
			}
		}

		//ブレンド係数を計算
		itr->blendFactor = itr->blendingTime == 0.0f ? 0.0f : (itr->blendingTime / itr->blendTime);
		const float lerpValue = itr->blendFactor;

		//RootMotionに使用するボーンの変化量を無効化するための値を計算
		{
			const float yaw = anim->getFrameCache()[rootMotionIndex].rotation.getYaw();
			Quaternion blendRootMotionRotate = Quaternion::euler({ 0,yaw,0 }, true);
			Vector3 blendRootMotionTranslate = anim->getFrameCache()[rootMotionIndex].position;
			blendRootMotionTranslate.y = 0;

			anim->rootMotionTranslate = blendRootMotionTranslate;
			anim->rootMotionRotate = blendRootMotionRotate; 
		}


		//RootMotionの変化量として扱う値を計算
		{
			TransformQ blendRootMotionLinerFirst = anim->getRootMotionTransform(false);
			TransformQ blendRootMotionLinerSecond = anim->getRootMotionTransform(true);

			//最終フレームと開始フレームをまたがった場合
			float yawRotate = blendRootMotionLinerFirst.rotation.getYaw() - blendRootMotionLinerSecond.rotation.getYaw();
			if (anim->getPlaingFrame() - anim->getBeforePlaingFrame() <= 0) {
				const auto& animBone = anim->getAnimationBones()[rootMotionIndex];
				const int maxKeyIndex = anim->getMaxFrame() - 1;

				const Vector3& startPos = animBone.keys[0].position;
				const Vector3& endPos = animBone.keys[maxKeyIndex].position;

				blendRootMotionLinerFirst.position += endPos - startPos;
				blendRootMotionLinerFirst.position.y = 0;

				const float startYaw = animBone.keys[0].rotation.getYaw();
				const float endYaw = animBone.keys[maxKeyIndex].rotation.getYaw();
				yawRotate += startYaw - endYaw;
			}

			//現在のアニメーションの変化量
			TransformQ blendRootMotionResult;
			blendRootMotionResult.position = blendRootMotionLinerFirst.position - blendRootMotionLinerSecond.position;
			blendRootMotionResult.rotation = Quaternion::euler({ 0,yawRotate,0 }, true);

			//アニメーションごとの蓄積する
			rootMotionVelocity.position = Vector3::lerp(blendRootMotionResult.position, rootMotionVelocity.position, lerpValue);
			rootMotionVelocity.rotation = Quaternion::slerp(blendRootMotionResult.rotation, rootMotionVelocity.rotation, lerpValue);
		}
	}

	for (int i = 0; i < _avator->getSize(); ++i) {

		Vector3 lerpPosition;
		Vector3 lerpScale;
		Quaternion slerpRotation;

		Vector3 rootMotionTranslate = Vector3::zero;
		Quaternion rootMotionRotate = Quaternion::identity;

		for (auto&& a : _duringAnimations) {

			auto anim = a.anim;
			TransformQ baseTransform = anim->getFrameCache()[i];
			const float lerpValue = a.blendFactor;

			//中間値で補完した各座標を取得
			lerpPosition = Vector3::lerp(baseTransform.position, lerpPosition, lerpValue);
			lerpScale = Vector3::lerp(baseTransform.scale, lerpScale, lerpValue);
			slerpRotation = Quaternion::slerp(baseTransform.rotation, slerpRotation, lerpValue);

			//ルートモーションが有効な場合は対象ボーンの逆行列の重みを計算
			if (applyRootMotion) {
				rootMotionTranslate = Vector3::lerp(anim->rootMotionTranslate, rootMotionTranslate, lerpValue);
				rootMotionRotate = Quaternion::slerp(anim->rootMotionRotate, rootMotionRotate, lerpValue);
			}
		}

		//行列に変換
		const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
		const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
		const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

		//行列合成
		const Matrix4 mtxRootMotionBlend = Matrix4::createWorldMatrix(rootMotionTranslate, rootMotionRotate, Vector3::one).inverse();
		const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);
		(*_avator->animatedPose)[i].matrix = matrix.multiply(mtxRootMotionBlend);

	}
}

void AnimationController::setRootMotionBone(const std::string & boneName) {
	for (int i = 0; i < _avator->getSize(); ++i) {
		if (_avator->bindPose->boneMatrices[i].name == boneName) {
			rootMotionIndex = i;
		}
	}
}
