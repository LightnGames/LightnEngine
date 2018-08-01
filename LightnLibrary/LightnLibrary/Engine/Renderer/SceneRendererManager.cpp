#include "SceneRendererManager.h"
#include "Mesh/StaticMesh.h"
#include "Mesh/SkeletalMesh.h"
#include "Mesh/DebugGeometry.h"
#include "RenderableEntity.h"
#include "LightEntity.h"

#include <Renderer/StaticInstancedMeshRenderer.h>
#include <Renderer/GameRenderer.h>

template<> SceneRendererManager* Singleton<SceneRendererManager>::mSingleton = 0;

SceneRendererManager::SceneRendererManager(){
	clearDebugGeometry();
}

SceneRendererManager::~SceneRendererManager() {
}

const RenderableEntityList & SceneRendererManager::renderableEntities() const {
	return _renderableEntities;
}

const LightEntityList & SceneRendererManager::lightEntities() const {
	return _lightEntities;
}

const DebugDrawList & SceneRendererManager::debugLines() const
{
	return _debugLines;
}

const DebugDrawList & SceneRendererManager::debugBoxs() const
{
	return _debugBoxs;
}

const DebugDrawList & SceneRendererManager::debugSpheres() const
{
	return _debugSpheres;
}

void SceneRendererManager::addRenderableEntity(RefPtr<RenderableEntity> renderableEntity) {
	_renderableEntities.emplace_back(renderableEntity);
}

void SceneRendererManager::removeRenderableEntity(const RefPtr<RenderableEntity>& renderableEntity) {
	_renderableEntities.remove(renderableEntity);
}

void SceneRendererManager::addLightEntity(RefPtr<LightEntity> lightEntity) {
	_lightEntities.emplace_back(lightEntity);
}

void SceneRendererManager::removeLightEntity(const RefPtr<LightEntity>& lightEntity) {
	_lightEntities.remove(lightEntity);
}

void SceneRendererManager::debugDrawLine(const Vector3 & start, const Vector3 & end, const Vector4 & color)
{
	const Vector3 startToEnd = end - start;
	const float length = startToEnd.length();

	Matrix4 mtxRotate = Matrix4::rotateAxis(Vector3::normalize(startToEnd));
	Matrix4 mtxTranslate = Matrix4::translateXYZ(start);
	Matrix4 mtxScale = Matrix4::scaleXYZ(length, length, length);

	DebugGeometoryInfo info;
	info.mtxWorld = Matrix4::transpose(mtxScale.multiply(mtxRotate.multiply(mtxTranslate)));
	info.color = color;
	instance()._debugLines.emplace_back(info);
}

void SceneRendererManager::debugDrawBox(const Vector3 & center, const Vector3 & extent, const Quaternion & rotate, const Vector4 & color)
{
	DebugGeometoryInfo info;
	info.mtxWorld = Matrix4::transpose(Matrix4::createWorldMatrix(center, rotate, extent));
	info.color = color;
	instance()._debugBoxs.emplace_back(info);
}

void SceneRendererManager::debugDrawSphere(const Vector3 & center, float radius, const Vector4 & color)
{
	DebugGeometoryInfo info;
	info.mtxWorld = Matrix4::transpose(Matrix4::createWorldMatrix(center, Quaternion::identity, Vector3::one*radius));
	info.color = color;
	instance()._debugSpheres.emplace_back(info);
}

void SceneRendererManager::clearDebugGeometry()
{
	_debugLines.clear();
	_debugSpheres.clear();
	_debugBoxs.clear();

	_debugLines.reserve(MAX_INSTANCE_NUM);
	_debugSpheres.reserve(MAX_INSTANCE_NUM);
	_debugBoxs.reserve(MAX_INSTANCE_NUM);
}

void SceneRendererManager::setUpStaticInstanceMeshRendere(uint32 maxDrawCount, const std::vector<uint32>& indexList) {
	StaticInstancedMeshRenderer::instance().initialize(GameRenderer::instance().device(), maxDrawCount, indexList);
}
