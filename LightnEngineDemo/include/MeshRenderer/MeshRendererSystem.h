#pragma once
#include <Core/Asset.h>
#include <GfxCore/GfxModuleSettings.h>
#include <MeshRenderer/GpuStruct.h>

struct CommandList;
struct ViewInfo;
struct Material;

struct MeshInfo {
	static constexpr u32 MATERIAL_SLOT_COUNT_MAX = 32;
	static constexpr u32 LOD_COUNT_MAX = 16;

	u32 _meshIndex = 0;
	u32 _vertexCount = 0;
	u32 _indexCount = 0;
	u32 _classicIndexCount = 0;
	u32 _primitiveCount = 0;
	u32 _vertexBinaryIndex = 0;
	u32 _primitiveBinaryIndex = 0;
	u32 _classicIndexBinaryIndex = 0;
	u32 _indexBinaryIndex = 0;
	u32 _lodMeshStartIndex = 0;
	u32 _subMeshStartIndex = 0;
	u32 _meshletStartIndex = 0;
	u32 _totalLodMeshCount = 0;
	u32 _totalSubMeshCount = 0;
	u32 _totalMeshletCount = 0;
	u32 _materialSlotCount = 0;
	u64 _materialSlotNameHashes[MATERIAL_SLOT_COUNT_MAX] = {};
	Material* _defaultMaterials[MATERIAL_SLOT_COUNT_MAX] = {};
	u32 _subMeshOffsets[LOD_COUNT_MAX] = {};
	Vector3 _boundsMin;
	Vector3 _boundsMax;
};

struct LodInfo {
};

struct SubMeshInfo {
	u32 _classiciIndexOffset = 0;
	u32 _classicIndexCount = 0;
	u32 _meshletOffset = 0;
	u32 _meshletCount = 0;
};

struct DebugMeshInfo {
	char _filePath[FILE_PATH_COUNT_MAX] = {};
	u64 _filePathHash = 0;
};

class LTN_MESH_RENDERER_API Mesh {
public:
	void requestToDelete();

	Asset* getAsset() { return _asset; }

	const MeshInfo* getMeshInfo() const {
		return _meshInfo;
	}

	const SubMeshInfo* getSubMeshInfo(u32 index = 0) const {
		return &_subMeshInfos[index];
	}

	const DebugMeshInfo* getDebugMeshInfo() const {
		return _debugMeshInfo;
	}

	const gpu::Mesh* getGpuMesh() const {
		return _gpuMesh;
	}

	const gpu::LodMesh* getGpuLodMesh(u32 index = 0) const {
		return &_gpuLodMeshes[index];
	}

	const gpu::SubMesh* getGpuSubMesh(u32 index = 0) const {
		return &_gpuSubMeshes[index];
	}

	const gpu::Meshlet* getGpuMeshlet(u32 index = 0) const {
		return &_gpuMeshlets[index];
	}

protected:
	Asset* _asset = nullptr;
	u8* _stateFlags = nullptr;
	const gpu::Mesh* _gpuMesh = nullptr;
	const gpu::LodMesh* _gpuLodMeshes = nullptr;
	const gpu::SubMesh* _gpuSubMeshes = nullptr;
	const gpu::Meshlet* _gpuMeshlets = nullptr;
	const MeshInfo* _meshInfo = nullptr;
	const SubMeshInfo* _subMeshInfos = nullptr;
	const DebugMeshInfo* _debugMeshInfo = nullptr;
};

class LTN_MESH_RENDERER_API SubMeshInstance {
public:
	virtual void setMaterial(Material* material) = 0;
	void setPrevMaterial(Material* material) { _prevMaterial = material; }

	Material* getMaterial() {
		return _material;
	}

	const Material* getMaterial() const {
		return _material;
	}

	const Material* getPrevMaterial() const {
		return _prevMaterial;
	}

protected:
	u8* _updateFlags = nullptr;
	Material* _material = nullptr;
	Material* _prevMaterial = nullptr;
};

class LTN_MESH_RENDERER_API MeshInstance {
public:
	void requestToDelete();
	void setWorldMatrix(const Matrix4& matrixWorld);
	void setMaterialSlotIndex(Material* material, u32 slotIndex);
	void setMaterial(Material* material, u64 slotNameHash);
	void setVisiblity(bool visible);
	u32 getMaterialSlotIndex(u64 slotNameHash) const;

	bool isVisible() const { return _visiblity; }
	const Mesh* getMesh() const { return _mesh; }
	SubMeshInstance* getSubMeshInstance(u32 index) { return &_subMeshInstances[index]; }
	Matrix4 getWorldMatrix() const { return _matrixWorld; }

	gpu::MeshInstance* getGpuMeshInstance() { return _gpuMeshInstance; }
	gpu::LodMeshInstance* getGpuLodMeshInstance(u32 lodLevel) { return &_gpuLodMeshinstances[lodLevel];}
	gpu::SubMeshInstance* getGpuSubMeshInstance(u32 subMeshIndex) { return &_gpuSubMeshInstances[subMeshIndex]; }

	const gpu::MeshInstance* getGpuMeshInstance() const { return _gpuMeshInstance; }
	const gpu::LodMeshInstance* getGpuLodMeshInstance(u32 lodLevel) const { return &_gpuLodMeshinstances[lodLevel]; }
	const gpu::SubMeshInstance* getGpuSubMeshInstance(u32 subMeshIndex) const { return &_gpuSubMeshInstances[subMeshIndex]; }

	bool isEnabled() const { return _stateFlags != nullptr; }

protected:
	Matrix4 _matrixWorld;
	bool _visiblity = false;
	u8* _stateFlags = nullptr;
	u8* _updateFlags = nullptr;

	gpu::MeshInstance* _gpuMeshInstance = nullptr;
	gpu::LodMeshInstance* _gpuLodMeshinstances = nullptr;
	gpu::SubMeshInstance* _gpuSubMeshInstances = nullptr;
	SubMeshInstance* _subMeshInstances = nullptr;
	const Mesh* _mesh = nullptr;
};

struct MeshDesc {
	const char* _filePath = nullptr;
};

struct MeshInstanceDesc {
	const Mesh** _meshes = nullptr;
	u32 _instanceCount = 1;
};

class LTN_MESH_RENDERER_API MeshRendererSystem {
public:
	virtual void initialize() = 0;
	virtual void update() = 0;
	virtual void terminate() = 0;
	virtual void processDeletion() = 0;
	virtual void render(CommandList* commandList, ViewInfo* viewInfo) = 0;
	virtual void renderDebugFixed(CommandList* commandList, ViewInfo* viewInfo) = 0;
	virtual Mesh* allocateMesh(const MeshDesc& desc) = 0;
	virtual Mesh* createMesh(const MeshDesc& desc) = 0;
	virtual void createMeshInstance(MeshInstance** outMeshInstances, const MeshInstanceDesc& desc) = 0;
	virtual Mesh* findMesh(u64 filePathHash) = 0;

	static MeshRendererSystem* Get();
};