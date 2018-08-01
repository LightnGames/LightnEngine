#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <Transform.h>

struct Skeleton;

struct AnimationBone{

	std::vector<TransformQ> keys;
	Matrix4 animatedMatrix;

	TransformQ& operator[](int boneIndex){
		return keys[boneIndex];
	}
};

class SkeltalAnimation{

public:

	~SkeltalAnimation();

	SkeltalAnimation(const std::string& fileName);

	void load(const std::string& fileName);

	//�A�j���[�V�����X�V
	void update(float deltaTime);

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

	//�ŗL���ʎq���擾
	std::string getName() const;

	//�A�j���[�V�����̃t���[���L���b�V�����擾
	const std::vector<TransformQ>& getFrameCache() const;
	
private:

	std::vector<AnimationBone> _animationBones;
	std::vector<TransformQ> _frameCache;
	std::string _name;

	int _maxFrame;
	float _plaingFrame;
	float _playRate;
};