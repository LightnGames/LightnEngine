#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <Util/Type.h>
#include <Transform.h>

struct Skeleton;

struct AnimationBone{

	std::vector<TransformQ> keys;
	Matrix4 animatedMatrix;

	TransformQ& operator[](int boneIndex){
		return keys[boneIndex];
	}
};

#define LOOP_BLEND_RANGE 0.1f

class SkeltalAnimation{

public:

	~SkeltalAnimation();

	SkeltalAnimation(const std::string& fileName);

	void load(const std::string& fileName);

	//�A�j���[�V�����X�V
	void update(float deltaTime, int rootMotionIndex = -1, float overrideFrame = -1.0f);

	//�A�j���[�V�����{�[���ϊ��s����擾
	const Matrix4& getAnimationMatrix(int boneIndex) const;

	//�A�j���[�V���������Z�b�g����
	void resetAnimation();

	//�Đ��X�s�[�h���w��
	void setPlayRate(float rate);

	//�Đ��X�s�[�h���擾
	float getPlayRate() const;

	//�A�j���[�V�����̍ő�t���[�����擾
	int getMaxFrame() const;

	float getPlaingFrame() const;
	float getBeforePlaingFrame() const;
	const std::vector<AnimationBone>& getAnimationBones() const;
	const TransformQ& getRootMotionTransform(bool second) const;

	//�ŗL���ʎq���擾
	std::string getName() const;

	//�A�j���[�V�����̃t���[���L���b�V�����擾
	const std::vector<TransformQ>& getFrameCache() const;
	
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