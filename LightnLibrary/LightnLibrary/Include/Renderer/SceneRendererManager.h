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

	//�`��ΏۃI�u�W�F�N�g���擾
	const RenderableEntityList& renderableEntities() const;
	const LightEntityList& lightEntities() const;
	const DebugDrawList& debugLines() const;
	const DebugDrawList& debugBoxs() const;
	const DebugDrawList& debugSpheres() const;
	const TileBasedPointLightType* pointLights(const Matrix4& mtxCamera) const;
	const TileBasedSpotLightType* spotLights(const Matrix4& mtxCamera) const;

	RefPtr<SkyBox> skyBox();

	//�`��ΏۃI�u�W�F�N�g��ǉ�
	void addRenderableEntity(RefPtr<RenderableEntity> renderableEntity);
	void addLightEntity(RefPtr<LightEntity> lightEntity);

	void addPointLight(RefPtr<PointLightComponent> light);
	void addSpotLight(RefPtr<SpotLightComponent> light);
	void removePointLight(RefPtr<PointLightComponent> light);
	void removeSpotLight(RefPtr<SpotLightComponent> light);

	//�`��ΏۃI�u�W�F�N�g�����X�g����O��
	void removeRenderableEntity(const RefPtr<RenderableEntity>& renderableEntity);
	void removeLightEntity(const RefPtr<LightEntity>& lightEntity);

	//�f�o�b�O�`��
	static void debugDrawLine(const Vector3& start, const Vector3& end, const Vector4& color = { 1,0,0,1 });
	static void debugDrawBox(const Vector3& center, const Vector3& extent = Vector3::one, const Quaternion& rotate = Quaternion::identity, const Vector4& color = { 1,0,0,1 });
	static void debugDrawSphere(const Vector3& center, float radius = 1.0f, const Vector4& color = { 1,0,0,1 });

	//�f�o�b�O�`�惊�X�g���폜
	void clearDebugGeometry();

	//StaticInstancedMeshRenderer��ݒ�E������
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