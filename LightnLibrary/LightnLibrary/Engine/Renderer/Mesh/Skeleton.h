#pragma once

#include <vector>
#include <string>
#include <Matrix4.h>
#include "../../Util/RefPtr.h"

//�{�[���\����
struct Bone{
	std::string name;
	Matrix4 matrix;
};

struct Skeleton{
	std::vector<Bone> boneMatrices;

	Skeleton(){}
	Skeleton(int boneCount):boneMatrices{boneCount}{}

	Bone& operator[](int boneIndex){
		return boneMatrices[boneIndex];
	}
};

struct Avator{
	std::unique_ptr<Skeleton> bindPose;
	std::unique_ptr<Skeleton> animatedPose;

	Avator(std::unique_ptr<Skeleton> bindPose)
		:bindPose{ std::move(bindPose) }, animatedPose{ std::make_unique<Skeleton>(this->bindPose->boneMatrices.size()) }{}

	~Avator() {}

	int getSize() const{
		return bindPose->boneMatrices.size();
	}
};