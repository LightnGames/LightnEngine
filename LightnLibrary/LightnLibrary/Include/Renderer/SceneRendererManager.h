#pragma once

#include <list>
#include <vector>
#include <LMath.h>
#include <Util/Singleton.h>
#include <Util/Util.h>

class StaticMesh;
class SkeletalMesh;
class RenderableEntity;
class LightEntity;
class InstanceStaticMesh;
class PointLightComponent;
class SpotLightComponent;
class SkyBox;

struct DebugGeometoryInfo;
struct TileBasedPointLightType;
struct TileBasedSpotLightType;

using RenderableEntityList = std::list<RefPtr<RenderableEntity>>;
using LightEntityList = std::list<RefPtr<LightEntity>>;
using DebugDrawList = std::vector<DebugGeometoryInfo>;
using StaticInstanceList = std::vector<RefPtr<InstanceStaticMesh>>;
using PointLightList = std::list<RefPtr<PointLightComponent>>;
using SpotLightList = std::list<RefPtr<SpotLightComponent>>;

class SceneRendererManager :public Singleton<SceneRendererManager>{

public:

	SceneRendererManager();

	~SceneRendererManager();

	void setUp();

	//描画対象オブジェクトを取得
	const RenderableEntityList& renderableEntities() const;
	const LightEntityList& lightEntities() const;
	const DebugDrawList& debugLines() const;
	const DebugDrawList& debugBoxs() const;
	const DebugDrawList& debugSpheres() const;
	const TileBasedPointLightType* pointLights(const Matrix4& mtxCamera) const;
	const TileBasedSpotLightType* spotLights(const Matrix4& mtxCamera) const;

	RefPtr<SkyBox> skyBox();

	//描画対象オブジェクトを追加
	void addRenderableEntity(RefPtr<RenderableEntity> renderableEntity);
	void addLightEntity(RefPtr<LightEntity> lightEntity);

	void addPointLight(RefPtr<PointLightComponent> light);
	void addSpotLight(RefPtr<SpotLightComponent> light);
	void removePointLight(RefPtr<PointLightComponent> light);
	void removeSpotLight(RefPtr<SpotLightComponent> light);

	//描画対象オブジェクトをリストから外す
	void removeRenderableEntity(const RefPtr<RenderableEntity>& renderableEntity);
	void removeLightEntity(const RefPtr<LightEntity>& lightEntity);

	//デバッグ描画
	static void debugDrawLine(const Vector3& start, const Vector3& end, const Vector4& color = { 1,0,0,1 });
	static void debugDrawBox(const Vector3& center, const Vector3& extent = Vector3::one, const Quaternion& rotate = Quaternion::identity, const Vector4& color = { 1,0,0,1 });
	static void debugDrawSphere(const Vector3& center, float radius = 1.0f, const Vector4& color = { 1,0,0,1 });

	//デバッグ描画リストを削除
	void clearDebugGeometry();

	//StaticInstancedMeshRendererを設定・初期化
	void setUpStaticInstanceMeshRendere(uint32 maxDrawCount, const std::vector<uint32>& indexList);

	RefPtr<SkyBox> getSkyBox();

private:

	RenderableEntityList _renderableEntities;
	LightEntityList _lightEntities;
	PointLightList _pointLightList;
	SpotLightList _spotLightList;

	DebugDrawList _debugSpheres;
	DebugDrawList _debugBoxs;
	DebugDrawList _debugLines;

	std::unique_ptr<TileBasedPointLightType> _pointLightListPtr;
	std::unique_ptr<TileBasedSpotLightType> _spotLightListPtr;

	//RefPtr<SkyBox> _skyBox;
	std::unique_ptr<SkyBox> _skyBox;
};