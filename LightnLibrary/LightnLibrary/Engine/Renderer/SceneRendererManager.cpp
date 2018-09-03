#include "SceneRendererManager.h"
#include "Mesh/StaticMesh.h"
#include "Mesh/SkeletalMesh.h"
#include "Mesh/DebugGeometry.h"
#include "Mesh/SkyBox.h"
#include "RenderableEntity.h"
#include "LightEntity.h"

#include <ShaderDefines.h>
#include <Renderer/Light/LightTypes.h>
#include <Renderer/StaticInstancedMeshRenderer.h>
#include <Renderer/GameRenderer.h>
#include <Components/PointLightComponent.h>
#include <Components/SpotLightComponent.h>
#include <Loader/MeshLoader.h>
#include <Renderer/RendererSettings.h>

template<> SceneRendererManager* Singleton<SceneRendererManager>::mSingleton = 0;

SceneRendererManager::SceneRendererManager() {
	clearDebugGeometry();
	TileBasedPointLightType* pointPtr = new TileBasedPointLightType[MAX_LIGHTS];
	TileBasedSpotLightType* spotPtr = new TileBasedSpotLightType[MAX_LIGHTS];
	_pointLightListPtr = std::unique_ptr<TileBasedPointLightType>(pointPtr);
	_spotLightListPtr = std::unique_ptr<TileBasedSpotLightType>(spotPtr);
}

SceneRendererManager::~SceneRendererManager() {
}

void SceneRendererManager::setUp() {

	auto& gameRenderer = GameRenderer::instance();

	std::vector<std::string> skyMatFiles;
	skyMatFiles.push_back("SkyBoxMaterial.mat");

	MeshLoader meshLoader(gameRenderer.device(), gameRenderer.deviceContext());
	meshLoader.load("SkyBox.mesh", skyMatFiles);

	_skyBox = std::make_unique<SkyBox>(std::move(meshLoader.getMeshes()), gameRenderer.device());
	RendererSettings::skyBox = _skyBox->material(0)->ppTextures[0];
}

const RenderableEntityList & SceneRendererManager::renderableEntities() const {
	return _renderableEntities;
}

const LightEntityList & SceneRendererManager::lightEntities() const {
	return _lightEntities;
}

const DebugDrawList & SceneRendererManager::debugLines() const {
	return _debugLines;
}

const DebugDrawList & SceneRendererManager::debugBoxs() const {
	return _debugBoxs;
}

const DebugDrawList & SceneRendererManager::debugSpheres() const {
	return _debugSpheres;
}

const TileBasedPointLightType* SceneRendererManager::pointLights(const Matrix4& mtxCamera) const {

	uint32 counter = 0;
	memset(_pointLightListPtr.get(), 0, sizeof(TileBasedPointLightType)*MAX_LIGHTS);

	for (auto&& p : _pointLightList) {
		_pointLightListPtr.get()[counter] = p->light;
		_pointLightListPtr.get()[counter].positionView = Matrix4::transform(p->getWorldPosition(), mtxCamera);

		++counter;
	}

	return _pointLightListPtr.get();
}

const TileBasedSpotLightType * SceneRendererManager::spotLights(const Matrix4 & mtxCamera) const {

	uint32 counter = 0;
	memset(_spotLightListPtr.get(), 0, sizeof(TileBasedSpotLightType)*MAX_LIGHTS);

	for (auto&& p : _spotLightList) {

		auto& ptr = _spotLightListPtr.get()[counter];
		ptr = p->light;
		ptr.positionView = Matrix4::transform(p->getWorldPosition(), mtxCamera);
		ptr.direction = p->forwardVector();

		++counter;
	}

	return _spotLightListPtr.get();
}

RefPtr<SkyBox> SceneRendererManager::skyBox() {
	return _skyBox.get();
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

void SceneRendererManager::addPointLight(RefPtr<PointLightComponent> light) {
	_pointLightList.emplace_back(light);
}

void SceneRendererManager::addSpotLight(RefPtr<SpotLightComponent> light) {
	_spotLightList.emplace_back(light);
}

void SceneRendererManager::removePointLight(RefPtr<PointLightComponent> light) {
	_pointLightList.remove(light);
}

void SceneRendererManager::removeSpotLight(RefPtr<SpotLightComponent> light) {
	_spotLightList.remove(light);
}

void SceneRendererManager::removeLightEntity(const RefPtr<LightEntity>& lightEntity) {
	_lightEntities.remove(lightEntity);
}

void SceneRendererManager::debugDrawLine(const Vector3 & start, const Vector3 & end, const Vector4 & color) {
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

void SceneRendererManager::debugDrawBox(const Vector3 & center, const Vector3 & extent, const Quaternion & rotate, const Vector4 & color) {
	DebugGeometoryInfo info;
	info.mtxWorld = Matrix4::transpose(Matrix4::createWorldMatrix(center, rotate, extent));
	info.color = color;
	instance()._debugBoxs.emplace_back(info);
}

void SceneRendererManager::debugDrawSphere(const Vector3 & center, float radius, const Vector4 & color) {
	DebugGeometoryInfo info;
	info.mtxWorld = Matrix4::transpose(Matrix4::createWorldMatrix(center, Quaternion::identity, Vector3::one*radius * 2));
	info.color = color;
	instance()._debugSpheres.emplace_back(info);
}

void SceneRendererManager::clearDebugGeometry() {
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
