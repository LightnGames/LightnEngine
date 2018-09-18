#pragma once

#include <Component/Component.h>
#include <Renderer/Mesh/StaticMesh.h>
#include <Renderer/RenderableInterface.h>

class StaticMeshComponent :public Component, public RenderableEntity {

public:

	StaticMeshComponent();

	~StaticMeshComponent();

	RefPtr<StaticMesh> staticMesh();

	//�X�^�e�B�b�N���b�V�����Z�b�g�A�b�v
	virtual void setUpStaticMesh(const std::string& filePath, const std::vector<std::string>& matFiles);

	void draw(const DrawSettings& drawSettings) override;

	void drawDepth(const DrawSettings& drawSettings) override;

private:

	RefPtr<StaticMesh> _staticMesh;
};