#pragma once

#include <Animation/AnimationBase.h>
#include <Util/Util.h>

class SkeletalAnimation;

using BlendSpace1DTask = std::pair<SkeletalAnimation, float>;

class BlendSpace1D :public AnimationBase {

public:
	BlendSpace1D();
	~BlendSpace1D();

	void load(const std::string& assetName) override;

	//�u�����h�X�y�[�X�ɃA�j���[�V������ǉ�
	void addBlendAnimation(const std::string& name, float blendSpace);

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

	void setBlendSpace(float value) { _blendSpace = value; }
	float getBlendSpace() const { return _blendSpace; }


private:

	std::vector<BlendSpace1DTask> _anims;
	float _blendSpace;

	std::vector<TransformQ> _frameCache;
	RefPtr<SkeletalAnimation> _firstAnim;
	RefPtr<SkeletalAnimation> _secondAnim;
	float _lerpValue;

	float _playRate;
	std::string _name;

};