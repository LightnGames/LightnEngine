#pragma once

#include "BlendSpace1D.h"

using BlendSpace2DTask = std::pair<BlendSpace1D, float>;

class BlendSpace2D :public AnimationBase {

public:

	void load(const std::string& assetName) override;

	//�u�����h�X�y�[�X�ɃA�j���[�V������ǉ�
	void addBlendAnimation(const std::string& name, float blendSpaceX, float blendSpaceY);

	//�u�����h�X�y�[�X�A�j���[�V�������\�z
	void setUpBlendSpace();

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

	TransformQ getRootMotionTransform(uint32 rootMotionIndex) const override;

	//�ŗL���ʎq���擾
	std::string getName() const override;

	//�A�j���[�V�����̃t���[���L���b�V�����擾
	const std::vector<TransformQ>& getFrameCache() const override;

	void setBlendSpace(float valueX, float valueY) { _blendSpaceX = valueX; _blendSpaceY = valueY; }
	Vector2 getBlendSpace() const { return Vector2(_blendSpaceX, _blendSpaceY); }

private:

	std::vector<BlendSpace2DTask> _blends;
	float _blendSpaceX;
	float _blendSpaceY;

	std::vector<TransformQ> _frameCache;
	RefPtr<BlendSpace1D> _firstBlendSpace;
	RefPtr<BlendSpace1D> _secondBlendSpace;
	float _lerpValue;

	float _playRate;
	std::string _name;
};