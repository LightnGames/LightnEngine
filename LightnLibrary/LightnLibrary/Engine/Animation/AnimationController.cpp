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

	//���߂ăA�j���[�V�������Đ������Ƃ��̓u�����h�������I��0(�����Đ�)
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

	//�A�j���[�V������S���v�Z���Ă���ēx�擾���Ă���̂Ń����������ɂ߂Ĉ���
	for (auto&& animTask : _duringAnimations) {

		auto anim = animTask.anim;
		float& blendingTime = animTask.blendingTime;
		float& blendTime = animTask.blendTime;
		float& blendFactor = animTask.blendFactor;

		//�e��A�j���[�V�������ԍX�V
		anim->update(deltaTime, rootMotionIndex, debugTime);
		blendingTime -= 0.016666f;
		blendingTime = std::fmax(blendingTime, 0);

		//�u�����h�W�����v�Z
		blendFactor = (blendingTime < FLT_EPSILON) ? 0.0f : (blendingTime / blendTime);

		//RootMotion�̕ω��ʂƂ��Ĉ����l���v�Z
		{
			TransformQ blendRootMotionLinerFirst = anim->getRootMotionTransform(false);
			TransformQ blendRootMotionLinerSecond = anim->getRootMotionTransform(true);

			//�ŏI�t���[���ƊJ�n�t���[�����܂��������ꍇ
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

			//���݂̃A�j���[�V�����̕ω���
			TransformQ blendRootMotionResult;
			blendRootMotionResult.position = blendRootMotionLinerFirst.position - blendRootMotionLinerSecond.position;
			blendRootMotionResult.rotation = Quaternion::euler({ 0,yawRotate,0 }, true);

			//�A�j���[�V�������Ƃ̒~�ς���
			rootMotionVelocity.position = Vector3::lerp(blendRootMotionResult.position, rootMotionVelocity.position, blendFactor);
			rootMotionVelocity.rotation = Quaternion::slerp(blendRootMotionResult.rotation, rootMotionVelocity.rotation, blendFactor);
		}
	}

	Matrix4 mtxRootMotionBlend = Matrix4::identity;

	for (int i = 0; i < _avator->getSize(); ++i) {

		Vector3 lerpPosition;
		Vector3 lerpScale;
		Quaternion slerpRotation;

		for (auto&& a : _duringAnimations) {

			auto anim = a.anim;
			TransformQ baseTransform = anim->getFrameCache()[i];
			const float lerpValue = a.blendFactor;

			//���Ԓl�ŕ⊮�����e���W���擾
			lerpPosition = Vector3::lerp(baseTransform.position, lerpPosition, lerpValue);
			lerpScale = Vector3::lerp(baseTransform.scale, lerpScale, lerpValue);
			slerpRotation = Quaternion::slerp(baseTransform.rotation, slerpRotation, lerpValue);
		}

		//���[�g���[�V�������̈ړ��ʂ𖳌�������t�s����v�Z
		if (i == rootMotionIndex) {
			Vector3 pp = lerpPosition;
			pp.y = 0;
			const float yaw = slerpRotation.getYaw();
			Quaternion pr = Quaternion::euler({ 0,yaw,0 }, true);
			mtxRootMotionBlend = Matrix4::createWorldMatrix(pp, pr, Vector3::one).inverse();
		}

		//�s��ɕϊ�
		const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
		const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
		const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

		//�s�񍇐�
		const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);
		(*_avator->animatedPose)[i].matrix = matrix;

	}

	//���[�g���[�V�����̈ړ��𖳌���
	for (int i = 0; i < _avator->getSize(); ++i) {
		const auto& matrix = (*_avator->animatedPose)[i].matrix;
		(*_avator->animatedPose)[i].matrix = matrix.multiply(mtxRootMotionBlend);
	}

	//�Đ����I�������A�j���[�V�������폜
	for (auto itr = _duringAnimations.begin(); itr != _duringAnimations.end(); ++itr) {

		//�A�j���[�V�������X�g�̈�ԍŏ��̃A�j���[�V�����͍폜���Ȃ�(�Đ����Ȃ̂�)
		if ((itr->blendingTime < FLT_EPSILON)&& (itr != _duringAnimations.begin())) {
			itr = _duringAnimations.erase(--itr);
			continue;
		}
	}
}

void AnimationController::setRootMotionBone(const std::string & boneName) {
	for (int i = 0; i < _avator->getSize(); ++i) {
		if (_avator->bindPose->boneMatrices[i].name == boneName) {
			rootMotionIndex = i;
		}
	}
}
