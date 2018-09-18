#include <Component/StaticInstanceMeshComponent.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/SceneRendererManager.h>
#include <Renderer/DrawSettings.h>
#include <Renderer/Mesh/StaticInstanceMesh.h>
#include <Renderer/StaticInstancedMeshRenderer.h>
#include <Renderer/Mesh/TerrainMesh.h>

StaticInstanceMeshComponent::StaticInstanceMeshComponent() {
	SceneRendererManager::instance().addRenderableEntity(this);
}

StaticInstanceMeshComponent::~StaticInstanceMeshComponent() {
	SceneRendererManager::instance().removeRenderableEntity(this);
}

void StaticInstanceMeshComponent::setUpMesh(
	const std::string & filePath,
	const std::vector<std::string>& matFiles,
	const std::vector<Matrix4>& matrices,
	uint32 meshDrawOffset,
	uint32 matrixBufferOffset) {

	_staticInstanceMesh = GraphicsResourceManager::instance().loadStaticInstanceMesh<StaticInstanceMesh>(filePath, matFiles, matrices, meshDrawOffset, matrixBufferOffset);
}

void StaticInstanceMeshComponent::draw(const DrawSettings & drawSettings) {
	RefPtr<StaticInstanceMeshData> data = StaticInstancedMeshRenderer::instance().getInstanceBuffers();
	_staticInstanceMesh->draw(drawSettings, data);
}

void StaticInstanceMeshComponent::drawDepth(const DrawSettings & drawSettings) {
	RefPtr<StaticInstanceMeshData> data = StaticInstancedMeshRenderer::instance().getInstanceBuffers();
	_staticInstanceMesh->drawDepth(drawSettings, data);
}

RefPtr<const LocalMesh> StaticInstanceMeshComponent::meshInfo() const {
	return _staticInstanceMesh->meshInfo();
}

void TerrainComponent::setUpMesh(const std::string & filePath, const std::vector<std::string>& matFiles, const std::vector<Matrix4>& matrices, uint32 meshDrawOffset, uint32 matrixBufferOffset) {
	_staticInstanceMesh = GraphicsResourceManager::instance().loadStaticInstanceMesh<TerrainMesh>(filePath, matFiles, matrices, meshDrawOffset, matrixBufferOffset).cast<StaticInstanceMesh>();
}
