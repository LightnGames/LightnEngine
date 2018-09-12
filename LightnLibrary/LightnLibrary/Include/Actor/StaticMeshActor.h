#pragma once

#include <Actor/Actor.h>
#include <Component/StaticMeshComponent.h>

class StaticMeshActor : public Actor {

public:

	StaticMeshActor();

	virtual void start() override;

	virtual void setUpTask() override;

	//�X�^�e�B�b�N���b�V�����Z�b�g�A�b�v
	virtual void setUpStaticMesh(const std::string& filePath, const std::vector<std::string>& matFiles);

	virtual void update(float deltaTime) override;

protected:

	RefPtr<StaticMeshComponent> _staticMeshComponent;

};