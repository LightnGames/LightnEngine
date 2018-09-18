#include <Animation/AnimationController.h>
#include <Animation/AnimationBase.h>
#include <Renderer/Mesh/Skeleton.h>
#include <ThirdParty/ImGui/imgui.h>

AnimationController::AnimationController(RefPtr<Avator> avator) 
	:_avator{ avator }, _blendingTime{ 0.0f }, _blendTime{ 0.0f } {
}

AnimationController::~AnimationController() {
}

void AnimationController::addAnimationList(std::unique_ptr<AnimationBase> animation) {
	_playList[animation->getName()] = std::move(animation);
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
	task.anim = _playList[name].get();
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
		anim->updateTimer(deltaTime, debugTime);
		anim->computeBones(rootMotionIndex);
		blendingTime -= 0.016666f;
		blendingTime = std::fmaxf(blendingTime, 0.0f);

		if (!applyRootMotion) {
			continue;
		}

		//�u�����h�W�����v�Z
		blendFactor = (approximately(blendingTime, 0.0f)) ? 0.0f : (blendingTime / blendTime);

		//RootMotion�̕ω��ʂƂ��Ĉ����l���v�Z
		{
			TransformQ blendRootMotionLinerFirst = anim->getRootMotionTransform(rootMotionIndex);

			//�A�j���[�V�������Ƃ̒~�ς���
			rootMotionVelocity.position = Vector3::lerp(blendRootMotionLinerFirst.position, rootMotionVelocity.position, blendFactor);
			rootMotionVelocity.rotation = Quaternion::slerp(blendRootMotionLinerFirst.rotation, rootMotionVelocity.rotation, blendFactor);
		}
	}

	Matrix4 mtxRootMotionBlend = Matrix4::identity;
	float blend=0;
	for (uint32 i = 0; i < _avator->getSize(); ++i) {

		Vector3 lerpPosition;
		Vector3 lerpScale;
		Quaternion slerpRotation;

		for (auto&& a : _duringAnimations) {

			auto anim = a.anim;
			const TransformQ& baseTransform = anim->getFrameCache()[i];
			const float lerpValue = a.blendFactor;
			blend = lerpValue;

			//���Ԓl�ŕ⊮�����e���W���擾
			lerpPosition = Vector3::lerp(baseTransform.position, lerpPosition, lerpValue);
			lerpScale = Vector3::lerp(baseTransform.scale, lerpScale, lerpValue);
			slerpRotation = Quaternion::slerp(baseTransform.rotation, slerpRotation, lerpValue);
		}

		//�s��ɕϊ�
		const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
		const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
		const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

		//�s�񍇐�
		const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);
		(*_avator->animatedPose)[i].matrix = matrix;

	}

	//ImGui::Begin("anim blend");
	//ImGui::SliderFloat("Value", &blend, -2, 2);
	//ImGui::End();

	//�Đ����I�������A�j���[�V�������폜
	for (auto itr = _duringAnimations.begin(); itr != _duringAnimations.end(); ++itr) {

		//�A�j���[�V�������X�g�̈�ԍŏ��̃A�j���[�V�����͍폜���Ȃ�(�Đ����Ȃ̂�)
		if (approximately(itr->blendingTime, 0.0f) && (itr != _duringAnimations.begin())) {
			itr = _duringAnimations.erase(--itr);
			continue;
		}
	}
}

void AnimationController::setRootMotionBone(const std::string & boneName) {
	for (uint32 i = 0; i < _avator->getSize(); ++i) {
		if (_avator->bindPose->boneMatrices[i].name == boneName) {
			rootMotionIndex = i;
		}
	}
}
