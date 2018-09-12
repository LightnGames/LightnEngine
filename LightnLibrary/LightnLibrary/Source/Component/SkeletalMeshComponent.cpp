#include <Component/SkeletalMeshComponent.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/SceneRendererManager.h>
#include <Renderer/DrawSettings.h>

#include <Actor/Actor.h>

SkeletalMeshComponent::SkeletalMeshComponent() {
	SceneRendererManager::instance().addRenderableEntity(this);
}

SkeletalMeshComponent::~SkeletalMeshComponent() {
	SceneRendererManager::instance().removeRenderableEntity(this);
}

const RefPtr<SkeletalMesh>& SkeletalMeshComponent::skeletalMesh() const {
	return _skeletalMesh;
}

void SkeletalMeshComponent::setUpSkeletalMesh(const std::string & filePath, const std::vector<std::string>& matFiles)
{
	_skeletalMesh = GraphicsResourceManager::instance().loadRenderableObject(filePath, matFiles).cast<SkeletalMesh>();
}

void SkeletalMeshComponent::update(float deltaTime) {
	Component::update(deltaTime);
}

void SkeletalMeshComponent::draw(const DrawSettings & drawSettings) {
	_skeletalMesh->draw(drawSettings, getWorldMatrix());
}

void SkeletalMeshComponent::drawDepth(const DrawSettings & drawSettings)
{
	_skeletalMesh->drawDepth(drawSettings, getWorldMatrix());
}
