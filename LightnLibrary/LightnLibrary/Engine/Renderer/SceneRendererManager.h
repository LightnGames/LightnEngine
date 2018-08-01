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

	//�`��ΏۃI�u�W�F�N�g���擾
	const RenderableEntityList& renderableEntities() const;
	const LightEntityList& lightEntities() const;
	const DebugDrawList& debugLines() const;
	const DebugDrawList& debugBoxs() const;
	const DebugDrawList& debugSpheres() const;

	//�`��ΏۃI�u�W�F�N�g��ǉ�
	void addRenderableEntity(RefPtr<RenderableEntity> renderableEntity);
	void addLightEntity(RefPtr<LightEntity> lightEntity);

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

private:

	RenderableEntityList _renderableEntities;
	LightEntityList _lightEntities;

	DebugDrawList _debugSpheres;
	DebugDrawList _debugBoxs;
	DebugDrawList _debugLines;

};