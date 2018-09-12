#include <Component/StaticMeshComponent.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/SceneRendererManager.h>
#include <Renderer/DrawSettings.h>

StaticMeshComponent::StaticMeshComponent() {
	SceneRendererManager::instance().addRenderableEntity(this);
}

StaticMeshComponent::~StaticMeshComponent() {
	SceneRendererManager::instance().removeRenderableEntity(this);
}

RefPtr<StaticMesh> StaticMeshComponent::staticMesh() {
	return _staticMesh;
}

void StaticMeshComponent::setUpStaticMesh(const std::string & filePath, const std::vector<std::string>& matFiles)
{
	_staticMesh = GraphicsResourceManager::instance().loadRenderableObject(filePath, matFiles).cast<StaticMesh>();
}

void StaticMeshComponent::draw(const DrawSettings & drawSettings) {
	_staticMesh->draw(drawSettings, getWorldMatrix());
}

void StaticMeshComponent::drawDepth(const DrawSettings & drawSettings)
{
	_staticMesh->drawDepth(drawSettings, getWorldMatrix());
}
