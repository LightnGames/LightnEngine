#pragma once

#include <Actor/Actor.h>

#include <Component/SkeletalMeshComponent.h>
#include <Component/AnimationControllerComponenth.h>
#include <Component/CameraComponent.h>

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