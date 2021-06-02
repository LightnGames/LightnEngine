#pragma once
#include <GfxCore/impl/GpuResourceImpl.h>
#include <MeshRenderer/MeshRendererSystem.h>
#include <MeshRenderer/GpuStruct.h>

enum MeshInstanceStateFlag {
	MESH_INSTANCE_FLAG_NONE = 0,
	MESH_INSTANCE_FLAG_SCENE_ENQUEUED = 1 << 0,
	MESH_INSTANCE_FLAG_SCENE_ENABLED = 1 << 1,
	MESH_INSTANCE_FLAG_REQUEST_DESTROY = 1 << 2,
};

enum MeshInstanceUpdateFlag {
	MESH_INSTANCE_UPDATE_NONE = 0,
	MESH_INSTANCE_UPDATE_WORLD_MATRIX = 1 << 0,
	MESH_INSTANCE_UPDATE_VISIBLE = 1 << 1,
};

enum SubMeshInstanceUpdateFlag {
	SUB_MESH_INSTANCE_UPDATE_NONE = 0,
	SUB_MESH_INSTANCE_UPDATE_MATERIAL = 1 << 1,
};

class PipelineStateGroup;
struct ViewInfo;
struct ShaderSet;

struct SceneCullingInfo {
	u32 _meshInstanceCountMax = 0;
};

struct HizInfoConstant {
	u32 _inputDepthWidth = 0;
	u32 _inputDepthHeight = 0;
	f32 _nearClip = 0.0f;
	f32 _farClip = 0.0f;
	u32 _inputBitDepth = 0;
};

namespace CullingResult {
	static f32 getPersentage(u32 passCount, u32 testCount) {
		if (passCount == 0 || testCount == 0) {
			return 0.0f;
		}
		return (passCount / static_cast<f32>(testCount)) * 100.0f;
	}

	static f32 getPassFrustumCullingMeshInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passFrustumCullingMeshInstanceCount, gpuCullingResult->_testFrustumCullingMeshInstanceCount);
	}

	static f32 getPassOcclusionCullingMeshInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingMeshInstanceCount, gpuCullingResult->_testOcclusionCullingMeshInstanceCount);
	}

	static f32 getPassSummaryCullingMeshInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingMeshInstanceCount, gpuCullingResult->_testFrustumCullingMeshInstanceCount);
	}

	static f32 getPassFrustumCullingSubMeshInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passFrustumCullingSubMeshInstanceCount, gpuCullingResult->_testFrustumCullingSubMeshInstanceCount);
	}

	static f32 getPassOcclusionCullingSubMeshInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingSubMeshInstanceCount, gpuCullingResult->_testOcclusionCullingSubMeshInstanceCount);
	}

	static f32 getPassSummaryCullingSubMeshInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingSubMeshInstanceCount, gpuCullingResult->_testFrustumCullingSubMeshInstanceCount);
	}

	static f32 getPassFrustumCullingMeshletInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passFrustumCullingMeshletInstanceCount, gpuCullingResult->_testFrustumCullingMeshletInstanceCount);
	}

	static f32 getPassBackfaceCullingMeshletInstancePersentage(const gpu::AmplificationCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passBackfaceCullingMeshletInstanceCount, gpuCullingResult->_testBackfaceCullingMeshletInstanceCount);
	}

	static f32 getPassNdcCullingMeshletInstancePersentage(const gpu::AmplificationCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passNdcCullingMeshletInstanceCount, gpuCullingResult->_testNdcCullingMeshletInstanceCount);
	}

	static f32 getPassOcclusionCullingMeshletInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingMeshletInstanceCount, gpuCullingResult->_testOcclusionCullingMeshletInstanceCount);
	}

	static f32 getPassSummaryCullingMeshletInstancePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingMeshletInstanceCount, gpuCullingResult->_testFrustumCullingMeshletInstanceCount);
	}

	static f32 getPassFrustumCullingTrianglePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passFrustumCullingTriangleCount, gpuCullingResult->_testFrustumCullingTriangleCount);
	}

	static f32 getPassBackfaceCullingTrianglePersentage(const gpu::AmplificationCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passBackfaceCullingTriangleCount, gpuCullingResult->_testBackfaceCullingTriangleCount);
	}

	static f32 getPassNdcCullingTrianglePersentage(const gpu::AmplificationCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passNdcCullingTriangleCount, gpuCullingResult->_testNdcCullingTriangleCount);
	}

	static f32 getPassOcclusionCullingTrianglePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingTriangleCount, gpuCullingResult->_testOcclusionCullingTriangleCount);
	}

	static f32 getPassSummaryCullingTrianglePersentage(const gpu::GpuCullingResult* gpuCullingResult) {
		return getPersentage(gpuCullingResult->_passOcclusionCullingTriangleCount, gpuCullingResult->_testFrustumCullingTriangleCount);
	}
};

class IndirectArgumentResource {
public:
	struct InitializeDesc {
		u32 _indirectArgumentCount = 0;
		u32 _indirectArgumentCounterCount = 0;
		u32 _strideInByte = 0;
	};

	void initialize(const InitializeDesc& desc);
	void terminate();

	void resourceBarriersToUav(CommandList* commandList);
	void resourceBarriersToIndirectArgument(CommandList* commandList);
	void resetIndirectArgumentCountBuffers(CommandList* commandList);
	void executeIndirect(CommandList* commandList, CommandSignature* commandSignature, u32 commandCountMax, u32 indirectArgumentOffset, u32 countBufferOffset);

	GpuBuffer* getIndirectArgumentBuffer() { return &_indirectArgumentBuffer; }
	GpuBuffer* getIndirectArgumentCountBuffer() { return &_countBuffer; }
	GpuDescriptorHandle getIndirectArgumentUav() const { return _indirectArgumentUavHandle._gpuHandle; }

private:
	u32 _indirectArgumentCountMax = 0;
	u32 _indirectArgumentCounterCountMax = 0;
	GpuBuffer _indirectArgumentBuffer;
	GpuBuffer _countBuffer;

	DescriptorHandle _indirectArgumentUavHandle;
	DescriptorHandle _countCpuUav;
};

class SubMeshInstanceImpl : public SubMeshInstance {
public:
	virtual void setMaterial(Material* material) override;

	// 参照カウンタを操作しない初期化専用のマテリアルセット
	void setDefaultMaterial(Material* material);

	void setUpdateFlags(u8* updateFlags) {
		_updateFlags = updateFlags;
	}
};

class MeshInstanceImpl : public MeshInstance {
public:
	void setEnabled() {
		*_stateFlags |= MESH_INSTANCE_FLAG_SCENE_ENABLED;
	}

	void setStateFlags(u8* stateFlags) {
		_stateFlags = stateFlags;
	}

	void setUpdateFlags(u8* updateFlags) {
		_updateFlags = updateFlags;
	}

	void setMesh(const Mesh* mesh) {
		_mesh = mesh;
	}

	void setGpuMeshInstance(gpu::MeshInstance* meshInstance) {
		_gpuMeshInstance = meshInstance;
	}

	void setGpuLodMeshInstances(gpu::LodMeshInstance* lodMeshInstances) {
		_gpuLodMeshinstances = lodMeshInstances;
	}

	void setGpuSubMeshInstances(gpu::SubMeshInstance* subMeshInstances) {
		_gpuSubMeshInstances = subMeshInstances;
	}

	void setSubMeshInstance(SubMeshInstance* subMeshInstances) {
		_subMeshInstances = subMeshInstances;
	}
};

class GpuCullingResource {
public:
	void initialize();
	void terminate();
	void update(const ViewInfo* viewInfo);
	void debugDrawHiz();

	GpuDescriptorHandle getCurrentLodLevelSrv() const { return _currentLodLevelSrv._gpuHandle; }
	ResourceDesc getHizTextureResourceDesc(u32 level) const;
	const gpu::GpuCullingResult* getGpuCullingResult() const;
	const gpu::AmplificationCullingResult* getAmplificationCullingResult() const;

	void setComputeLodResource(CommandList* commandList);
	void setGpuCullingResources(CommandList* commandList);
	void setHizResourcesPass0(CommandList* commandList);
	void setHizResourcesPass1(CommandList* commandList);
	void resourceBarriersComputeLodToUAV(CommandList* commandList);
	void resetResourceComputeLodBarriers(CommandList* commandList);
	void resourceBarriersHizTextureToUav(CommandList* commandList, u32 offset);
	void resourceBarriersHizUavtoSrv(CommandList* commandList, u32 offset);
	void resourceBarriersHizSrvToTexture(CommandList* commandList);
	void resourceBarriersHizTextureToSrv(CommandList* commandList);
	void resetResultBuffers(CommandList* commandList);
	void readbackCullingResultBuffer(CommandList* commandList);
	void setDrawResultDescriptorTable(CommandList* commandList);
	void setDrawCurrentLodDescriptorTable(CommandList* commandList);

private:
	GpuBuffer _currentLodLevelBuffer;
	GpuBuffer _gpuCullingResultBuffer;
	GpuBuffer _gpuCullingResultReadbackBuffer;
	GpuBuffer _amplificationCullingResultBuffer;
	GpuBuffer _amplificationCullingResultReadbackBuffer;
	GpuBuffer _hizInfoConstantBuffer[2];
	GpuTexture _hizDepthTextures[gpu::HIERACHICAL_DEPTH_COUNT] = {};

	gpu::GpuCullingResult _currentFrameGpuCullingResult;
	gpu::AmplificationCullingResult _currentFrameAmplificationCullingResult;
	DescriptorHandle _hizDepthTextureSrv;
	DescriptorHandle _hizDepthTextureUav;
	DescriptorHandle _hizInfoConstantCbv[2];
	DescriptorHandle _gpuCullingResultUavHandle;
	DescriptorHandle _gpuCullingResultCpuUavHandle;
	DescriptorHandle _amplificationCullingResultUavHandle;
	DescriptorHandle _amplificationCullingResultCpuUavHandle;
	DescriptorHandle _currentLodLevelUav;
	DescriptorHandle _currentLodLevelSrv;
};

class InstancingResource {
public:
	static constexpr u32 INSTANCING_PER_SHADER_COUNT_MAX = 1024 * 4; // SUB_MESH_COUNT_MAX
	static constexpr u32 INDIRECT_ARGUMENT_COUNT_MAX = INSTANCING_PER_SHADER_COUNT_MAX * gpu::SHADER_SET_COUNT_MAX;
	static constexpr u32 INDIRECT_ARGUMENT_COUNTER_COUNT_MAX = INSTANCING_PER_SHADER_COUNT_MAX * gpu::SHADER_SET_COUNT_MAX;

	struct UpdateDesc {
		MeshInstanceImpl* _meshInstances = nullptr;
		u32 _meshletThresholdUseAmplificationShader = 0;
		u32 _countMax = 0;
	};

	void initialize();
	void terminate();
	void update(const UpdateDesc& desc);

	void resetInfoCountBuffers(CommandList* commandList);
	void resourceBarriersGpuCullingToUAV(CommandList* commandList);
	void resetResourceGpuCullingBarriers(CommandList* commandList);

	const u16* getMsIndirectArgumentCounts() const { return _msIndirectArgumentCounts; }
	const u16* getAsMsIndirectArgumentCounts() const { return _asMsIndirectArgumentCounts; }
	GpuDescriptorHandle getInfoOffsetSrv() const { return _infoOffsetSrv._gpuHandle; }
	GpuDescriptorHandle getInfoCountUav() const { return _infoCountUav._gpuHandle; }
	GpuDescriptorHandle getInfoCountSrv() const { return _infoCountSrv._gpuHandle; }
	GpuDescriptorHandle getMeshInstanceIndexUav() const { return _primitiveInfoMeshInstanceIndexUav._gpuHandle; }
	GpuDescriptorHandle getMeshInstanceIndexSrv() const { return _primitiveInfoMeshInstanceIndexSrv._gpuHandle; }
	GpuDescriptorHandle getPrimitiveInfoUav() const { return _primitiveInfoUav._gpuHandle; }
	GpuDescriptorHandle getPrimitiveInfoSrv() const { return _primitiveInfoSrv._gpuHandle; }

private:
	GpuBuffer _primitiveInfoBuffer;
	GpuBuffer _primitiveInfoMeshInstanceIndexBuffer;
	GpuBuffer _infoCountBuffer;
	GpuBuffer _infoOffsetBuffer;
	DescriptorHandle _infoOffsetSrv;
	DescriptorHandle _infoCountCpuUav;
	DescriptorHandle _infoCountUav;
	DescriptorHandle _infoCountSrv;
	DescriptorHandle _primitiveInfoUav;
	DescriptorHandle _primitiveInfoSrv;
	DescriptorHandle _primitiveInfoMeshInstanceIndexUav;
	DescriptorHandle _primitiveInfoMeshInstanceIndexSrv;
	u16 _msIndirectArgumentCounts[gpu::SHADER_SET_COUNT_MAX] = {};
	u16 _asMsIndirectArgumentCounts[gpu::SHADER_SET_COUNT_MAX] = {};
};

#if ENABLE_MULTI_INDIRECT_DRAW
class MultiDrawInstancingResource {
public:
	struct UpdateDesc {
		const gpu::SubMeshInstance* _subMeshInstances = nullptr;
		u32 _countMax = 0;
	};

	void initialize();
	void terminate();
	void update(const UpdateDesc& desc);

	const u32* getIndirectArgumentCounts() const { return _indirectArgumentCounts; }
	const u32* getIndirectArgumentOffsets() const { return _indirectArgumentOffsets; }
	GpuDescriptorHandle getIndirectArgumentOffsetSrv() const { return _indirectArgumentOffsetSrv._gpuHandle; }

private:
	u32 _indirectArgumentOffsets[gpu::SHADER_SET_COUNT_MAX] = {};
	u32 _indirectArgumentCounts[gpu::SHADER_SET_COUNT_MAX] = {};
	GpuBuffer _indirectArgumentOffsetBuffer;
	DescriptorHandle _indirectArgumentOffsetSrv;
};
#endif

class BuildIndirectArgumentResource {
public:
	struct UpdateDesc {
		u32 _packedMeshletCount = 0;
	};

	struct Constant {
		u32 _packedMeshletCount = 0;
	};

	void initialize();
	void terminate();
	void update(const UpdateDesc& desc);

	GpuDescriptorHandle getConstantCbv() const { return _constantCbv._gpuHandle; }

private:
	GpuBuffer _constantBuffer;
	DescriptorHandle _constantCbv;
};

struct SceneInfo {
	u32 _meshInstanceCount = 0;
	u32 _lodMeshInstanceCount = 0;
	u32 _subMeshInstanceCount = 0;
	u32 _meshletInstanceCount = 0;
	u32 _vertexCount = 0;
	u32 _triangleCount = 0;
};

class Scene {
public:
	static constexpr u32 MESH_INSTANCE_COUNT_MAX = 1024 * 16;
	static constexpr u32 LOD_MESH_INSTANCE_COUNT_MAX = 1024 * 64;
	static constexpr u32 SUB_MESH_INSTANCE_COUNT_MAX = 1024 * 256;
	static constexpr u32 MESHLET_INSTANCE_MESHLET_COUNT_MAX = 64;
	static constexpr u32 MESHLET_INSTANCE_INFO_COUNT_MAX = (MESHLET_INSTANCE_MESHLET_COUNT_MAX + 1) * gpu::SHADER_SET_COUNT_MAX;

	void initialize();
	void update();
	void processDeletion();
	void terminate();
	void terminateDefaultResources();
	void uploadMeshInstance(u32 meshInstanceIndex);
	void deleteMeshInstance(u32 meshInstanceIndex);
	void debugDrawMeshInstanceBounds();
	void debugDrawGui();

	bool isUpdatedInstancingOffset() const { return _isUpdatedInstancingOffset; }
	MeshInstanceImpl* getMeshInstance(u32 index) { return &_meshInstances[index]; }
	void createMeshInstances(MeshInstance** outMeshInstances, const Mesh** meshes, u32 instanceCount);
	GpuDescriptorHandle getMeshInstanceSrv() const { return _meshInstanceSrv._gpuHandle; }
	GpuDescriptorHandle getSceneCbv() const { return _cullingSceneConstantHandle._gpuHandle; }
	GpuDescriptorHandle getMeshInstanceWorldMatrixSrv() const { return _meshInstanceWorldMatrixSrv._gpuHandle; }
	u32 getMeshInstanceCountMax() const { return MESH_INSTANCE_COUNT_MAX; }
	u32 getMeshInstanceCount() const { return _gpuMeshInstances.getInstanceCount(); }
	u32 getMeshInstanceArrayCountMax() const { return _gpuMeshInstances.getResarveCount(); }
	u32 getSubMeshInstanceArrayCountMax() const { return _gpuSubMeshInstances.getResarveCount(); }
	const gpu::SubMeshInstance* getSubMeshInstances() const { return &_gpuSubMeshInstances[0]; }
	const u8* getMeshInstanceStateFlags() const { return _meshInstanceStateFlags; }
	SceneInfo getSceneInfo() const { return _sceneInfo; }
	SceneInfo getVisibleSceneInfo() const { return _visibleSceneInfo; }

private:
	u8 _meshInstanceStateFlags[MESH_INSTANCE_COUNT_MAX] = {};
	u8 _meshInstanceUpdateFlags[MESH_INSTANCE_COUNT_MAX] = {};
	u8 _subMeshInstanceUpdateFlags[SUB_MESH_INSTANCE_COUNT_MAX] = {};
	MeshInstanceImpl _meshInstances[MESH_INSTANCE_COUNT_MAX] = {};
	SubMeshInstanceImpl _subMeshInstances[SUB_MESH_INSTANCE_COUNT_MAX] = {};

	MultiDynamicQueue<gpu::MeshInstance> _gpuMeshInstances;
	MultiDynamicQueue<gpu::LodMeshInstance> _gpuLodMeshInstances;
	MultiDynamicQueue<gpu::SubMeshInstance> _gpuSubMeshInstances;

	GpuBuffer _meshInstanceBuffer;
	GpuBuffer _meshInstanceWorldMatrixBuffer;
	GpuBuffer _lodMeshInstanceBuffer;
	GpuBuffer _subMeshInstanceBuffer;
	GpuBuffer _sceneCullingConstantBuffer;

	DescriptorHandle _cullingSceneConstantHandle;
	DescriptorHandle _meshInstanceSrv;
	DescriptorHandle _meshInstanceWorldMatrixSrv;
	Material* _defaultMaterial = nullptr;
	ShaderSet* _defaultShaderSet = nullptr;
	bool _isUpdatedInstancingOffset = false;

	SceneInfo _sceneInfo;
	SceneInfo _visibleSceneInfo;
};