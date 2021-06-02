#pragma once
#include <MeshRenderer/MeshRendererSystem.h>
#include <MeshRenderer/GpuStruct.h>
#include <MeshRenderer/impl/SceneImpl.h>
#include <MeshRenderer/impl/MeshResourceManager.h>
#include <MeshRenderer/impl/MeshRenderer.h>
#include <MeshRenderer/impl/VramShaderSetSystem.h>

class PipelineStateGroup;
class PipelineStateSet;
struct MeshRenderDesc {
	CommandList* _commandList = nullptr;
};

class MeshRendererSystemImpl :public MeshRendererSystem {
public:
	virtual void initialize() override;
	virtual void terminate() override;
	virtual void update() override;
	virtual void render(CommandList* commandList, ViewInfo* viewInfo) override;
	virtual void renderDebugFixed(CommandList* commandList, ViewInfo* viewInfo) override;
	virtual void processDeletion() override;

	virtual Mesh* allocateMesh(const MeshDesc& desc) override;
	virtual Mesh* createMesh(const MeshDesc& desc) override;
	virtual void createMeshInstance(MeshInstance** outMeshInstances, const MeshInstanceDesc& desc) override;
	virtual Mesh* findMesh(u64 filePathHash) override;

	static MeshRendererSystemImpl* Get();
private:
#if ENABLE_MESH_SHADER
	void renderMeshShader(CommandList* commandList, ViewInfo* viewInfo);
	void renderMeshShaderDebugFixed(CommandList* commandList, ViewInfo* viewInfo);
#endif
#if ENABLE_MULTI_INDIRECT_DRAW
	void renderMultiIndirect(CommandList* commandList, ViewInfo* viewInfo);
	void renderMultiIndirectDebugFixed(CommandList* commandList, ViewInfo* viewInfo);
#endif
#if ENABLE_CLASSIC_VERTEX
	void renderClassicVertex(CommandList* commandList, ViewInfo* viewInfo);
#endif
private:
	PipelineStateGroup** getPipelineStateGroup(PipelineStateSet* pipelineStateSet);
	void setupDraw(CommandList* commandList, ViewInfo* viewInfo);
	void debugDrawGpuCullingResult();
	void debugDrawAmplificationCullingResult();
	void updateVisiblityHighMeshes(s32 visibleType);

private:
	bool _visible = true;
	Scene _scene;
	MeshResourceManager _resourceManager;
	MeshRenderer _meshRenderer;
	VramShaderSetSystem _vramShaderSetSystem;
	InstancingResource _instancingResource;
	IndirectArgumentResource _indirectArgumentResource;
	IndirectArgumentResource _primIndirectArgumentResource;
	GpuCullingResource _gpuCullingResource;
	BuildIndirectArgumentResource _buildIndirectArgumentResource;

#if ENABLE_MULTI_INDIRECT_DRAW
	MultiDrawInstancingResource _multiDrawInstancingResource;
	IndirectArgumentResource _multiDrawIndirectArgumentResource;
	VertexBufferView _vertexBufferViews[2] = {};
	IndexBufferView _indexBufferView = {};
#endif

	enum GeometoryType {
		GEOMETORY_MODE_MESH_SHADER = 0,
		GEOMETORY_MODE_MULTI_INDIRECT,
		GEOMETORY_MODE_CLASSIC_VERTEX,
	};

	enum DebugPrimitiveType {
		DEBUG_PRIMITIVE_TYPE_DEFAULT = 0,
		DEBUG_PRIMITIVE_TYPE_MESHLET,
		DEBUG_PRIMITIVE_TYPE_LODLEVEL,
		DEBUG_PRIMITIVE_TYPE_OCCLUSION,
		DEBUG_PRIMITIVE_TYPE_DEPTH,
		DEBUG_PRIMITIVE_TYPE_TEXCOORDS,
		DEBUG_PRIMITIVE_TYPE_WIREFRAME,
	};

	enum CullingDebugMenuType {
		CULLING_DEBUG_TYPE_NONE = 0,
		CULLING_DEBUG_TYPE_PASS_MESH_CULLING = 1 << 0,
		CULLING_DEBUG_TYPE_PASS_MESHLET_CULLING = 1 << 1
	};

	void setDebugCullingFlag(u8 type, bool flag) {
		_cullingDebugFlags = flag ? _cullingDebugFlags | type : _cullingDebugFlags & ~type;
	}

	u32 _packedMeshletCount = 0;
	bool _debugDrawMeshletBounds = false;
	GeometoryType _geometryType;
	DebugPrimitiveType _debugPrimitiveType;
	u8 _cullingDebugFlags = CULLING_DEBUG_TYPE_NONE;
};