#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <Util/Util.h>
#include <Transform.h>

struct Skeleton;

struct AnimationBone {

	std::vector<TransformQ> keys;

	TransformQ& operator[](int boneIndex) {
		return keys[boneIndex];
	}
};


class AnimationBase {

public:

	AnimationBase() {};

	virtual ~AnimationBase() {};

	virtual void load(const std::string& fileName) = 0;

	//�A�j���[�V�����^�C�}�X�V
	virtual void updateTimer(float deltaTime, float overrideFrame = -1.0f) = 0;

	//�A�j���[�V�����X�V
	virtual void computeBones(int32 rootMotionIndex = -1) = 0;

	//�A�j���[�V���������Z�b�g����
	virtual void resetAnimation() = 0;

	//�Đ��X�s�[�h���w��
	virtual void setPlayRate(float rate) = 0;

	//�Đ��X�s�[�h���擾
	virtual float getPlayRate() const = 0;

	virtual TransformQ getRootMotionTransform(uint32 rootMotionIndex) const = 0;

	//�ŗL���ʎq���擾
	virtual std::string getName() const = 0;

	//�A�j���[�V�����̃t���[���L���b�V�����擾
	virtual const std::vector<TransformQ>& getFrameCache() const = 0;

};