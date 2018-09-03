#include "AnimationController.h"
#include "SkeltalAnimation.h"
#include "../Renderer/Mesh/Skeleton.h"
#include <ThirdParty/ImGui/imgui.h>

AnimationController::AnimationController(RefPtr<Avator> avator) :_avator{ avator }, _blendingTime{ 0.0f }, _blendTime{ 0.0f }{
}

AnimationController::~AnimationController(){
}

void AnimationController::addAnimationList(const Animation & animation){
	_playList[animation->getName()] = animation;
}

void AnimationController::addAnimationList(const std::string & name){
	auto anim = std::make_shared<SkeltalAnimation>(name);
	addAnimationList(anim);
}

void AnimationController::play(const std::string & name, float blendTime){

	if(_playList.count(name) == 0){
		return;
	}

	//前回のアニメーションがあればフレームをリセット
	if(_duringAnimation.lock() != nullptr){
		_blendingAnimation = _playList[name];
		_blendingAnimation.lock()->resetAnimation();
	}
	else{
		_duringAnimation = _playList[name];
	}

	_blendingTime = blendTime;
	_blendTime = blendTime;
}

void AnimationController::update(float deltaTime){

	auto anim = _duringAnimation.lock();
	if(anim == nullptr){
		return;
	}

	//アニメーションを全部計算してから再度取得しているのでメモリ効率極めて悪し
	anim->update(deltaTime, rootMotionIndex, debugTime);

	Matrix4 mtxRootMotion = Matrix4::identity;
	TransformQ rootMotionResult;
	Vector3 rootMotionTranslate;
	Quaternion rootMotionRotate;

	if (applyRootMotion) {
		const float yaw = anim->getFrameCache()[rootMotionIndex].rotation.getYaw();
		rootMotionRotate = Quaternion::euler({ 0,yaw,0 }, true);
		rootMotionTranslate = anim->getFrameCache()[rootMotionIndex].position;
		rootMotionTranslate.y = 0;
		mtxRootMotion = Matrix4::createWorldMatrix(rootMotionTranslate, rootMotionRotate, Vector3::one).inverse();

		TransformQ rootMotionLinerFirst = anim->getRootMotionTransform(false);
		TransformQ rootMotionLinerSecond = anim->getRootMotionTransform(true);
		float yawRotate = rootMotionLinerFirst.rotation.getYaw() - rootMotionLinerSecond.rotation.getYaw();

		//最終フレームと開始フレームをまたがった場合
		if (anim->getPlaingFrame() - anim->getBeforePlaingFrame() <= 0) {

			const auto& animBone = anim->getAnimationBones()[rootMotionIndex];
			const int maxKeyIndex = anim->getMaxFrame() - 1;

			const Vector3& startPos = animBone.keys[0].position;
			const Vector3& endPos = animBone.keys[maxKeyIndex].position;

			rootMotionLinerFirst.position += endPos - startPos;
			rootMotionLinerFirst.position.y = 0;

			const float startYaw = animBone.keys[0].rotation.getYaw();
			const float endYaw = animBone.keys[maxKeyIndex].rotation.getYaw();
			yawRotate += startYaw - endYaw;
		}

		rootMotionResult.position = rootMotionLinerFirst.position - rootMotionLinerSecond.position;
		rootMotionResult.rotation = Quaternion::euler({ 0,yawRotate,0 }, true);
	}

	//ブレンド対象のアニメーションが存在する場合両方計算して合成
	auto blendingAnim = _blendingAnimation.lock();
	if(blendingAnim != nullptr){
		
		const float lerpValue = 1.0f - _blendingTime / _blendTime;
		blendingAnim->update(deltaTime, rootMotionIndex, debugTime);

		Matrix4 mtxRootMotionBlend = Matrix4::identity;
		if (applyRootMotion) {
			const float yaw = blendingAnim->getFrameCache()[rootMotionIndex].rotation.getYaw();
			Quaternion blendRootMotionRotate = Quaternion::euler({ 0,yaw,0 }, true);
			Vector3 blendRootMotionTranslate = blendingAnim->getFrameCache()[rootMotionIndex].position;
			blendRootMotionTranslate.y = 0;

			const Vector3 p = Vector3::lerp(rootMotionTranslate, blendRootMotionTranslate, lerpValue);
			const Quaternion pq = Quaternion::slerp(rootMotionRotate, blendRootMotionRotate, lerpValue);
			mtxRootMotionBlend = Matrix4::createWorldMatrix(p, pq, Vector3::one).inverse();

			//最終フレームと開始フレームをまたがった場合
			TransformQ blendRootMotionLinerFirst = blendingAnim->getRootMotionTransform(false);
			TransformQ blendRootMotionLinerSecond = blendingAnim->getRootMotionTransform(true);

			float yawRotate = blendRootMotionLinerFirst.rotation.getYaw() - blendRootMotionLinerSecond.rotation.getYaw();
			if (blendingAnim->getPlaingFrame() - blendingAnim->getBeforePlaingFrame() <= 0) {
				const auto& animBone = blendingAnim->getAnimationBones()[rootMotionIndex];
				const int maxKeyIndex = blendingAnim->getMaxFrame() - 1;

				const Vector3& startPos = animBone.keys[0].position;
				const Vector3& endPos = animBone.keys[maxKeyIndex].position;

				blendRootMotionLinerFirst.position += endPos - startPos;
				blendRootMotionLinerFirst.position.y = 0;

				const float startYaw = animBone.keys[0].rotation.getYaw();
				const float endYaw = animBone.keys[maxKeyIndex].rotation.getYaw();
				yawRotate += startYaw - endYaw;
			}

			TransformQ blendRootMotionResult;
			blendRootMotionResult.position = blendRootMotionLinerFirst.position - blendRootMotionLinerSecond.position;
			blendRootMotionResult.rotation = Quaternion::euler({ 0,yawRotate,0 }, true);
			
			rootMotionResult.position = Vector3::lerp(rootMotionResult.position, blendRootMotionResult.position, lerpValue);
			rootMotionResult.rotation = Quaternion::slerp(rootMotionResult.rotation, blendRootMotionResult.rotation, lerpValue);
		}

		for(int i = 0; i < _avator->getSize(); ++i){

			const TransformQ baseTransform = anim->getFrameCache()[i];
			const TransformQ blendTransform = blendingAnim->getFrameCache()[i];

			//中間値で補完した各座標を取得
			//しばらくは線形補完
			const Vector3 lerpPosition = Vector3::lerp(baseTransform.position, blendTransform.position, lerpValue);
			const Vector3 lerpScale = Vector3::lerp(baseTransform.scale, blendTransform.scale, lerpValue);
			const Quaternion slerpRotation = Quaternion::slerp(baseTransform.rotation, blendTransform.rotation, lerpValue);

			//行列に変換
			const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
			const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
			const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

			//行列合成
			const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);
			( *_avator->animatedPose )[i].matrix = matrix.multiply(mtxRootMotionBlend);
		}

		//ブレンドタイム更新
		_blendingTime -= 0.0166f;

		//
		if (_blendingTime <= 0.0f) {
			_duringAnimation = _blendingAnimation;
			_blendingAnimation.reset();
			_blendingTime = 0.0f;
		}

	}
	else {

		//ブレンド対象がない際のアニメーション更新
		for (int i = 0; i < _avator->getSize(); ++i) {
			(*_avator->animatedPose)[i].matrix = anim->getAnimationMatrix(i).multiply(mtxRootMotion);
		}
	}


	rootMotionVelocity = rootMotionResult;
	//rootMotionTranslate = Vector3::zero;

	ImGui::Begin("aaaaa");
	float yyy = rootMotionResult.rotation.getYaw()*(180.0f / M_PI);
	ImGui::SliderFloat("ZZZ ", &rootMotionVelocity.position.x, - 100, 100);
	ImGui::End();

}

void AnimationController::setRootMotionBone(const std::string & boneName) {
	for (int i = 0; i < _avator->getSize(); ++i) {
		if (_avator->bindPose->boneMatrices[i].name == boneName) {
			rootMotionIndex = i;
		}
	}
}
