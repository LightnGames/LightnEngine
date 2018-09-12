#pragma once

#include <Component/Component.h>
#include <Renderer/Mesh/SkeletalMesh.h>
#include <Renderer/RenderableEntity.h>

class SkeletalMeshComponent :public Component, public RenderableEntity{

public:

	SkeletalMeshComponent();

	~SkeletalMeshComponent();

	const RefPtr<SkeletalMesh>& skeletalMesh() const;

	//スケルタルメッシュをセットアップ
	virtual void setUpSkeletalMesh(const std::string& filePath, const std::vector<std::string>& matFiles);

	virtual void update(float deltaTime) override;

	void draw(const DrawSettings& drawSettings) override;

	void drawDepth(const DrawSettings& drawSettings) override;

private:

	RefPtr<SkeletalMesh> _skeletalMesh;
};