#pragma once

#include "Actor.h"

#include <Components/SkeletalMeshComponent.h>
#include <Components/AnimationControllerComponenth.h>
#include <Components/CameraComponent.h>

class SkeletalMeshActor : public Actor {

public:

	SkeletalMeshActor();

	void start() override;

	void setUpTask() override;

	//スケルタルメッシュをセットアップ
	void setUpSkeletalMesh(const std::string& filePath, const std::vector<std::string>& matFiles);

	void update(float deltaTime) override;

	RefPtr<SkeletalMeshComponent> _skeletalMeshComponent;
	RefPtr<AnimationControllerComponent> _animationComponent;
	RefPtr<CameraComponent> _camera;
};