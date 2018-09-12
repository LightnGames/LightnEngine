#pragma once

#include <vector>
#include <string>
#include <Matrix4.h>
#include <Util/Util.h>

//É{Å[Éìç\ë¢ëÃ
struct Bone{
	std::string name;
	Matrix4 matrix;
};

struct Skeleton{
	std::vector<Bone> boneMatrices;

	Skeleton(){}
	Skeleton(uint32 boneCount):boneMatrices{boneCount}{}

	Bone& operator[](uint32 boneIndex){
		return boneMatrices[boneIndex];
	}
};

struct Avator{
	std::unique_ptr<Skeleton> bindPose;
	std::unique_ptr<Skeleton> animatedPose;

	Avator(std::unique_ptr<Skeleton> bindPose)
		:bindPose{ std::move(bindPose) }, animatedPose{ std::make_unique<Skeleton>(static_cast<uint32>(this->bindPose->boneMatrices.size())) }{}

	~Avator() {}

	uint32 getSize() const {
		return static_cast<uint32>(bindPose->boneMatrices.size());
	}
};