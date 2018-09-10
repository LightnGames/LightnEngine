#pragma once

#include "AnimationBase.h"

class SkeletalAnimation :public AnimationBase {

public:

	SkeletalAnimation();

	~SkeletalAnimation();

	void load(const std::string& fileName) override;

	//�A�j���[�V�����^�C�}�X�V
	void updateTimer(float deltaTime, float overrideFrame = -1.0f);

	//�A�j���[�V�����X�V
	void computeBones(int32 rootMotionIndex = -1) override;

	//�A�j���[�V���������Z�b�g����
	void resetAnimation() override;

	//�Đ��X�s�[�h���w��
	void setPlayRate(float rate) override;

	//�Đ��X�s�[�h���擾
	float getPlayRate() const override;

	//�A�j���[�V�����̍ő�t���[�����擾
	int32 getMaxFrame() const;

	float getPlaingFrame() const;
	float getBeforePlaingFrame() const;
	const std::vector<AnimationBone>& getAnimationBones() const;
	TransformQ getRootMotionTransform(uint32 rootMotionIndex) const override;

	//�ŗL���ʎq���擾
	std::string getName() const override;

	//�A�j���[�V�����̃t���[���L���b�V�����擾
	const std::vector<TransformQ>& getFrameCache() const override;

private:

	Matrix4 getRootMotionMatrixInverse(uint32 rootMotionIndex) const;
	TransformQ getBoneTransformFromKey(uint32 boneIndex, uint32 keyIndex) const;
	void clampRootMotionTransform(TransformQ& transform) const;

private:

	std::vector<AnimationBone> _animationBones;
	std::vector<TransformQ> _frameCache;
	TransformQ _rootMotionTransform;
	TransformQ _rootMotionTransformSecond;
	std::string _name;

	uint32 _maxFrame;
	float _plaingFrame;
	float _beforePlaingFrame;
	float _playRate;
};