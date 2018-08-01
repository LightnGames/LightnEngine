#pragma once

#include <list>
#include <vector>
#include <LMath.h>
#include <Util/Singleton.h>
#include <Util/Type.h>

class StaticMesh;
class SkeletalMesh;
class RenderableEntity;
class LightEntity;
class InstanceStaticMesh;

struct DebugGeometoryInfo;

using RenderableEntityList = std::list<RefPtr<RenderableEntity>>;
using LightEntityList = std::list<RefPtr<LightEntity>>;
using DebugDrawList = std::vector<DebugGeometoryInfo>;
using StaticInstanceList = std::vector<RefPtr<InstanceStaticMesh>>;

class SceneRendererManager :public Singleton<SceneRendererManager>{

public:

	SceneRendererManager();

	~SceneRendererManager();

	//描画対象オブジェクトを取得
	const RenderableEntityList& renderableEntities() const;
	const LightEntityList& lightEntities() const;
	const DebugDrawList& debugLines() const;
	const DebugDrawList& debugBoxs() const;
	const DebugDrawList& debugSpheres() const;

	//描画対象オブジェクトを追加
	void addRenderableEntity(RefPtr<RenderableEntity> renderableEntity);
	void addLightEntity(RefPtr<LightEntity> lightEntity);

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

private:

	RenderableEntityList _renderableEntities;
	LightEntityList _lightEntities;

	DebugDrawList _debugSpheres;
	DebugDrawList _debugBoxs;
	DebugDrawList _debugLines;

};