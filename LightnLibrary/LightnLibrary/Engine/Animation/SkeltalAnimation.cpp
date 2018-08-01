#include "SkeltalAnimation.h"
#include <Loader/MeshLoader.h>
#include <Renderer/Mesh/Skeleton.h>
#include <fstream>
#include <algorithm>
#include <iostream>


SkeltalAnimation::SkeltalAnimation(const std::string& fileName) :_maxFrame{ 0 }, _plaingFrame{ 0.0f }, _playRate{ 1.0f }{

	//�t�@�C�����������ɂ���΃��[�h����
	if(fileName != ""){
		load(fileName);
	}
}

SkeltalAnimation::~SkeltalAnimation(){
}

void SkeltalAnimation::load(const std::string& fileName){

	//�t�@�C��������g���q�������ČŗL���ʎq�ɂ���
	int path_i = fileName.find_last_of("/") + 1;
	_name = fileName.substr(path_i);

	path_i = _name.find_last_of(".");
	_name = _name.substr(0, path_i);

	//  ios::in �͓ǂݍ��ݐ�p  ios::binary �̓o�C�i���`��
	std::ifstream fin(fileName, std::ios::in | std::ios::binary);

	if(!fin){
		std::cout << "�t�@�C�� " << fileName << " ���J���܂���";
	}

	//�{�[���T�C�Y
	int boneSize = 0;
	fin.read(reinterpret_cast<char*>( &boneSize ), sizeof(int));

	//�L�[�T�C�Y
	int keySize = 0;
	fin.read(reinterpret_cast<char*>( &keySize ), sizeof(int));

	//�ő�t���[����
	fin.read(reinterpret_cast<char*>( &_maxFrame ), sizeof(int));

	//�{�[�����������������m��
	_animationBones.reserve(boneSize);
	_frameCache.reserve(boneSize);
	_frameCache = std::vector<TransformQ>(boneSize);

	//�{�[�����ׂēǂݏI���܂Ń��[�v
	while(_animationBones.size() != boneSize){

		AnimationBone bone;

		//�{�[�����̃L�[�t���[����ǂݏI���܂Ń��[�v
		while(bone.keys.size() != keySize){

			//�L�[�t���[���\���̂��o�C�i���f�[�^���琶��
			TransformQ key;

			fin.read(reinterpret_cast<char*>( &key.position ), sizeof(Vector3));
			fin.read(reinterpret_cast<char*>( &key.rotation ), sizeof(Quaternion));
			fin.read(reinterpret_cast<char*>( &key.scale ), sizeof(Vector3));

			bone.keys.emplace_back(key);
		}

		_animationBones.emplace_back(bone);
	}

	fin.close();
}

void SkeltalAnimation::update(float deltaTime){

	//�t���[���X�V
	//�t�Đ��ɂ͖��Ή�
	_plaingFrame += 0.5f * getPlayRate();

	if(_plaingFrame >= _maxFrame){
		_plaingFrame -= _maxFrame;
	}

	for(int i = 0; i < _animationBones.size(); i++){

		//�ǂ̃t���[���Ԃ����ׂ�
		int firstFrame = static_cast<int>( std::floorf(_plaingFrame) );
		int secondFrame = static_cast<int>( std::ceilf(_plaingFrame) );

		//�t���[���̒��Ԓl(0.0f~1.0f)
		const float lerpValue = _plaingFrame - firstFrame;

		//�ŏI�t���[�����Q�Ƃ�����0�ɖ߂�
		if(secondFrame == _maxFrame){
			secondFrame = 0;
		}

		//�Q�Ƃ���L�[���擾
		const TransformQ firstkey = _animationBones[i][firstFrame];
		const TransformQ secondKey = _animationBones[i][secondFrame];

		//���Ԓl�ŕ⊮�����e���W���擾
		const Vector3 lerpPosition = Vector3::lerp(firstkey.position, secondKey.position, lerpValue);
		const Vector3 lerpScale = Vector3::lerp(firstkey.scale, secondKey.scale, lerpValue);
		const Quaternion slerpRotation = Quaternion::slerp(firstkey.rotation, secondKey.rotation, lerpValue);

		//�s��ɕϊ�
		const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
		const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
		const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

		//�s�񍇐�
		const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);

		//�ŏI�A�j���[�V�����ϊ��s����i�[
		_animationBones[i].animatedMatrix = matrix;

		//�⊮�����{�[�������L���b�V������
		const TransformQ transformCache(lerpPosition, slerpRotation, lerpScale);
		_frameCache[i] = transformCache;
	}
}

const Matrix4& SkeltalAnimation::getAnimationMatrix(int boneIndex) const{
	return _animationBones[boneIndex].animatedMatrix;
}

void SkeltalAnimation::resetAnimation(){
	_plaingFrame = 0;
}

void SkeltalAnimation::setPlayRate(float rate){
	_playRate = rate;
}

float SkeltalAnimation::getPlayRate() const{
	return _playRate;
}

int SkeltalAnimation::getMaxFrame() const{
	return _maxFrame;
}

std::string SkeltalAnimation::getName() const{
	return _name;
}

const std::vector<TransformQ>& SkeltalAnimation::getFrameCache() const{
	return _frameCache;
}
