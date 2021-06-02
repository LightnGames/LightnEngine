#pragma once
#include <MaterialSystem/MaterialSystem.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
struct PipelineState;
struct RootSignature;
struct CommandSignature;

enum PipelineStateGrpupFlags {
	PIPELINE_STATE_GROUP_FLAG_NONE = 0,
	PIPELINE_STATE_GROUP_FLAG_REQUEST_DESTROY = 1 << 0,
};

namespace DefaultMeshRootParam {
	enum {
		CULLING_VIEW_CONSTANT = 0,
		VIEW_CONSTANT,
		MATERIALS,
		MESH,
		MESH_INSTANCE,
		INDIRECT_CONSTANT,
		MESHLET_INFO,
		VERTEX_RESOURCES,
		TEXTURES,
		LOD_LEVEL,
		MESHLET_PRIMITIVE_INFO,
		MESHLET_MESH_INSTANCE_INDEX,
		MESH_INSTANCE_WORLD_MATRIX,
		CULLING_RESULT,
		HIZ,
		ROOT_DEFAULT_MESH_COUNT
	};

};

namespace ClassicMeshRootParam {
	enum {
		SCENE_CONSTANT = 0,
		MESH_INFO,
		MATERIALS,
		MESH_INSTANCE,
		TEXTURES,
		COUNT
	};
};

struct MeshShaderPipelineStateGroupDesc {
	ComparisonFunc _depthComparisonFunc;
	BlendDesc _blendDesc;
	FillMode _fillMode = FILL_MODE_SOLID;
	const char* _meshShaderFilePath = nullptr;
	const char* _amplificationShaderFilePath = nullptr;
	const char* _pixelShaderFilePath = nullptr;
};

struct ClassicPipelineStateGroupDesc {
	ComparisonFunc _depthComparisonFunc;
	BlendDesc _blendDesc;
	FillMode _fillMode = FILL_MODE_SOLID;
	const char* _vertexShaderFilePath = nullptr;
	const char* _pixelShaderFilePath = nullptr;
};

struct SharedRootSignature {
	u32 _refCount = 0;
	RootSignature* _rootSignature = nullptr;
};

class PipelineStateGroup {
public:
	static constexpr u32 MATERIAL_STRUCT_COUNT_MAX = 64;
	void initialize(const MeshShaderPipelineStateGroupDesc& desc, SharedRootSignature* rootSignature);
	void initialize(const ClassicPipelineStateGroupDesc& desc, SharedRootSignature* rootSignature);
	void terminate();
	void requestToDestroy();
	PipelineState* getPipelineState() { return _pipelineState; }
	RootSignature* getRootSignature() { return _sharedRootSignature->_rootSignature; }
	SharedRootSignature* getSharedRootSignature() { return _sharedRootSignature; }

	void setStateFlags(u8* flags) { _stateFlags = flags; }

private:
	u8* _stateFlags = nullptr;
	PipelineState* _pipelineState = nullptr;
	SharedRootSignature* _sharedRootSignature = nullptr;
};

class LTN_MATERIAL_SYSTEM_API PipelineStateSystem {
public:
	static constexpr u32 PIPELINE_STATE_GROUP_COUNT_MAX = 64;
	void initialize();
	void update();
	void processDeletion();
	void terminate();

	PipelineStateGroup* getGroup(u32 index) { return &_pipelineStates[index]; }
	u64 getPipelineStateGrpupHash(const PipelineStateGroup* group) const;
	u32 getGroupArrayCount() const { return _pipelineStates.getResarveCount(); }
	u32 getGroupIndex(const PipelineStateGroup* pipelineState) const;
	PipelineStateGroup* createPipelineStateGroup(const MeshShaderPipelineStateGroupDesc& desc, const RootSignatureDesc& rootSignatureDesc);
	PipelineStateGroup* createPipelineStateGroup(const ClassicPipelineStateGroupDesc& desc, const RootSignatureDesc& rootSignatureDesc);
	const u8* getStateArray() const { return _stateFlags; }

	static PipelineStateSystem* Get();
private:
	u32 createSharedRootSignature(const RootSignatureDesc& desc);
	u64 createRootSignatureDescHash(const RootSignatureDesc& desc) const;
	u32 findPipelineStateGroup(u64 hash) const;
	u32 findSharedRootsignature(u64 hash) const;

private:
	DynamicQueue<PipelineStateGroup> _pipelineStates;
	DynamicQueue<SharedRootSignature> _sharedRootsignatures;
	u64 _sharedRootSignatureHashes[PIPELINE_STATE_GROUP_COUNT_MAX] = {};
	u64 _pipelineStateHashes[PIPELINE_STATE_GROUP_COUNT_MAX] = {};
	u8 _stateFlags[PIPELINE_STATE_GROUP_COUNT_MAX] = {};
	u16 _refCounts[PIPELINE_STATE_GROUP_COUNT_MAX] = {};
};