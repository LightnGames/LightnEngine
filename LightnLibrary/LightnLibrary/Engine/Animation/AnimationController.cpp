#include "AnimationController.h"
#include "SkeltalAnimation.h"
#include "../Renderer/Mesh/Skeleton.h"

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

	//�O��̃A�j���[�V����������΃t���[�������Z�b�g
	if(_duringAnimation.lock() != nullptr){
		_duringAnimation.lock()->resetAnimation();
		_blendingAnimation = _playList[name];
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

	//�A�j���[�V������S���v�Z���Ă���ēx�擾���Ă���̂Ń����������ɂ߂Ĉ���
	anim->update(deltaTime);

	//�u�����h�Ώۂ̃A�j���[�V���������݂���ꍇ�����v�Z���č���
	auto blendingAnim = _blendingAnimation.lock();
	if(blendingAnim != nullptr){

		blendingAnim->update(deltaTime);

		for(int i = 0; i < _avator->getSize(); ++i){

			TransformQ baseTransform = anim->getFrameCache()[i];
			TransformQ blendTransform = blendingAnim->getFrameCache()[i];

			float lerpValue = 1.0f - _blendingTime / _blendTime;

			//���Ԓl�ŕ⊮�����e���W���擾
			//���΂炭�͐��`�⊮
			const Vector3 lerpPosition = Vector3::lerp(baseTransform.position, blendTransform.position, lerpValue);
			const Vector3 lerpScale = Vector3::lerp(baseTransform.scale, blendTransform.scale, lerpValue);
			const Quaternion slerpRotation = Quaternion::slerp(baseTransform.rotation, blendTransform.rotation, lerpValue);

			//�s��ɕϊ�
			const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
			const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
			const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

			//�s�񍇐�
			const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);
			( *_avator->animatedPose )[i].matrix = matrix;
		}

		//�u�����h�^�C���X�V
		_blendingTime -= 0.0166f;

		if(_blendingTime <= 0){
			_duringAnimation = _blendingAnimation;
			_blendingAnimation.reset();
			_blendingTime = 0.0f;
		}

		return;
	}

	//�u�����h�Ώۂ��Ȃ��ۂ̃A�j���[�V�����X�V
	for(int i = 0; i < _avator->getSize(); ++i){

		( *_avator->animatedPose )[i].matrix = anim->getAnimationMatrix(i);
	}

}
