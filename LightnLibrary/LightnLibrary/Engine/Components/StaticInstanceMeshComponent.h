#pragma once

#include "Component.h"
#include <vector>
#include <Renderer/RenderableEntity.h>

class StaticInstanceMesh;
struct LocalMesh;

class StaticInstanceMeshComponent :public Component, public RenderableEntity {

public:

	StaticInstanceMeshComponent();

	~StaticInstanceMeshComponent();

	virtual void setUpMesh(const std::string& filePath, const std::vector<std::string>& matFiles,
		const std::vector<Matrix4>& matrices, uint32 meshDrawOffset, uint32 matrixBufferOffset);

	void draw(const DrawSettings& drawSettings) override;

	void drawDepth(const DrawSettings& drawSettings) override;

	RefPtr<const LocalMesh> meshInfo() const;

private:

	RefPtr<StaticInstanceMesh> _staticInstanceMesh;
};