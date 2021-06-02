#pragma once
#include <Core/System.h>
#include <GfxCore/impl/GpuResourceImpl.h>
#include <GfxCore/impl/DescriptorHeap.h>

struct ViewInfo;
class IndirectArgumentResource;
class VramShaderSet;
class InstancingResource;
class GpuCullingResource;
class BuildIndirectArgumentResource;
class Scene;

struct RenderContext {
	CommandList* _commandList = nullptr;
	ViewInfo* _viewInfo = nullptr;
	VramShaderSet* _vramShaderSets = nullptr;
	IndirectArgumentResource* _indirectArgumentResource = nullptr;
	IndirectArgumentResource* _primIndirectArgumentResource = nullptr;
	InstancingResource* _instancingResource = nullptr;
	GpuCullingResource* _gpuCullingResource = nullptr;
	PipelineStateGroup** _primInstancingPipelineStates = nullptr;
	PipelineStateGroup** _pipelineStates = nullptr;
	CommandSignature** _primCommandSignatures = nullptr;
	CommandSignature** _commandSignatures = nullptr;
	GpuDescriptorHandle _meshSrv;
	GpuDescriptorHandle _vertexResourceDescriptors;
	GpuDescriptorHandle _debugFixedViewCbv;
	const Scene* _scene = nullptr;
	bool _collectResult = false;
};

struct ComputeLodContext {
	CommandList* _commandList = nullptr;
	ViewInfo* _viewInfo = nullptr;
	GpuCullingResource* _gpuCullingResource = nullptr;
	u32 _meshInstanceCountMax = 0;
	GpuDescriptorHandle _meshInstanceHandle;
	GpuDescriptorHandle _meshHandle;
	GpuDescriptorHandle _sceneConstantCbv;
};

struct GpuCullingContext {
	CommandList* _commandList = nullptr;
	GpuCullingResource* _gpuCullingResource = nullptr;
	InstancingResource* _instancingResource = nullptr;
	GpuDescriptorHandle _indirectArgumentOffsetSrv;
	GpuDescriptorHandle _sceneConstantCbv;
	GpuDescriptorHandle _meshInstanceSrv;
	GpuDescriptorHandle _meshSrv;
	GpuDescriptorHandle _subMeshDrawInfoSrv;
	GpuDescriptorHandle _cullingViewCbv;
	GpuDescriptorHandle _materialInstanceIndexSrv;
	u32 _meshInstanceCountMax = 0;
	bool _passCulling = false;
	const char* _scopeName = nullptr;
};

#if ENABLE_MULTI_INDIRECT_DRAW
struct MultiIndirectRenderContext {
	CommandList* _commandList = nullptr;
	ViewInfo* _viewInfo = nullptr;
	GpuCullingResource* _gpuCullingResource = nullptr;
	IndirectArgumentResource* _indirectArgumentResource = nullptr;
	VramShaderSet* _vramShaderSets = nullptr;
	PipelineStateGroup** _pipelineStates = nullptr;
	CommandSignature** _commandSignatures = nullptr;
	VertexBufferView* _vertexBufferViews = nullptr;
	IndexBufferView* _indexBufferView = nullptr;
	u32 _numVertexBufferView = 0;
	const u32* _indirectArgmentOffsets = nullptr;
	const u32* _indirectArgmentCounts = nullptr;
	GpuDescriptorHandle _meshInstanceWorldMatrixSrv;
};

struct MultiDrawGpuCullingContext {
	GpuCullingResource* _gpuCullingResource = nullptr;
	IndirectArgumentResource* _indirectArgumentResource = nullptr;
	CommandList* _commandList = nullptr;
	GpuDescriptorHandle _indirectArgumentOffsetSrv;
	GpuDescriptorHandle _sceneConstantCbv;
	GpuDescriptorHandle _meshInstanceSrv;
	GpuDescriptorHandle _meshSrv;
	GpuDescriptorHandle _subMeshDrawInfoHandle;
	GpuDescriptorHandle _cullingViewCbv;
	GpuDescriptorHandle _materialInstanceIndexSrv;
	u32 _meshInstanceCountMax = 0;
	bool _passCulling = false;
	const char* _scopeName = nullptr;
};
#endif

struct BuildIndirectArgumentContext {
	CommandList* _commandList = nullptr;
	IndirectArgumentResource* _indirectArgumentResource = nullptr;
	IndirectArgumentResource* _primIndirectArgumentResource = nullptr;
	BuildIndirectArgumentResource* _buildResource = nullptr;
	GpuDescriptorHandle _meshletInstanceOffsetSrv;
	GpuDescriptorHandle _meshletInstanceCountSrv;
	GpuDescriptorHandle _subMeshSrv;
};

struct BuildHizContext {
	CommandList* _commandList = nullptr;
	ViewInfo* _viewInfo = nullptr;
	GpuCullingResource* _gpuCullingResource = nullptr;
};

struct BuildDebugDrawMeshletBoundsContext {
	CommandList* _commandList = nullptr;
	GpuDescriptorHandle _sceneConstantCbv;
	GpuDescriptorHandle _meshInstanceSrv;
	GpuDescriptorHandle _meshInstanceWorldMatrixSrv;
	GpuDescriptorHandle _meshSrv;
	GpuDescriptorHandle _currentLodLevelSrv;
	u32 _meshInstanceCountMax = 0;
};

class MeshRenderer {
public:
	void initialize();
	void terminate();

	void render(const RenderContext& context) const;
	void computeLod(const ComputeLodContext& context) const;
	void depthPrePassCulling(const GpuCullingContext& context) const;
	void mainCulling(const GpuCullingContext& context) const;
	void buildIndirectArgument(const BuildIndirectArgumentContext& context) const;
	void buildHiz(const BuildHizContext& context) const;
	void buildDebugDrawBounds(const BuildDebugDrawMeshletBoundsContext& context) const;

#if ENABLE_MULTI_INDIRECT_DRAW
	void multiDrawRender(const MultiIndirectRenderContext& context) const;
	void multiDrawDepthPrePassCulling(const MultiDrawGpuCullingContext& context) const;
	void multiDrawMainCulling(const MultiDrawGpuCullingContext& context) const;
#endif

private:
	void setMeshShaderResources(const RenderContext& context, VramShaderSet* vramShaderSet) const;
	void gpuCulling(const GpuCullingContext& context, PipelineState* pipelineState) const;
#if ENABLE_MULTI_INDIRECT_DRAW
	void gpuCulling(const MultiDrawGpuCullingContext& context, PipelineState* pipelineState) const;
#endif

private:
	RootSignature* _gpuCullingRootSignature = nullptr;
	PipelineState* _gpuCullingPassPipelineState = nullptr;
	PipelineState* _gpuOcclusionCullingPipelineState = nullptr;
	PipelineState* _gpuCullingPipelineState = nullptr;

	PipelineState* _computeLodPipelineState = nullptr;
	RootSignature* _computeLodRootSignature = nullptr;
	PipelineState* _buildHizPipelineState = nullptr;
	RootSignature* _buildHizRootSignature = nullptr;
	PipelineState* _debugMeshletBoundsPipelineState = nullptr;
	RootSignature* _debugMeshletBoundsRootSignature = nullptr;
	PipelineState* _buildIndirectArgumentPipelineState = nullptr;
	RootSignature* _buildIndirectArgumentRootSignature = nullptr;
#if ENABLE_MULTI_INDIRECT_DRAW
	PipelineState* _multiDrawCullingPassPipelineState = nullptr;
	PipelineState* _multiDrawCullingPipelineState = nullptr;
	PipelineState* _multiDrawOcclusionCullingPipelineState = nullptr;
#endif
};