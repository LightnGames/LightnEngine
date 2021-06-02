#pragma once
#include <Core/System.h>

namespace gpu {
	constexpr u32 INVALID_INDEX = 0xffffffff;
	constexpr u32 HIERACHICAL_DEPTH_COUNT = 8;
	constexpr u32 SHADER_SET_COUNT_MAX = 32;

	enum MeshInstanceStateFlgas {
		MESH_INSTANCE_STATE_FLAGS_NONE = 0,
		MESH_INSTANCE_STATE_FLAGS_ENABLE = 1,
		MESH_INSTANCE_STATE_FLAGS_VISIBLE = 1 << 1,
		MESH_INSTANCE_STATE_FLAGS_CAST_SHADOW = 1 << 2,
	};

	enum MeshState {
		MESH_STATE_NONE = 0,
		MESH_STATE_ALLOCATED,
		MESH_STATE_LOADED,
	};

	struct Mesh {
		u32 _stateFlags = 0;
		u32 _lodMeshOffset = 0;
		u32 _lodMeshCount = 0;
	};

	struct LodMesh {
		u32 _vertexOffset = 0;
		u32 _vertexIndexOffset = 0;
		u32 _primitiveOffset = 0;
		u32 _subMeshOffset = 0;
		u32 _subMeshCount = 0;
	};

	struct SubMesh {
		u32 _meshletOffset = 0;
		u32 _meshletCount = 0;
		u32 _materialSlotIndex = 0;
	};

	struct SubMeshDrawInfo {
		u32 _indexCount = 0;
		u32 _indexOffset = 0;
	};

	struct Meshlet {
		Float3 _aabbMin;
		Float3 _aabbMax;
		u32 _normalAndCutoff = 0;
		f32 _apexOffset = 0;
	};

	struct MeshletPrimitiveInfo {
		u32 _vertexOffset = 0;
		u32 _vertexCount = 0;
		u32 _primitiveOffset = 0;
		u32 _primitiveCount = 0;
	};

	struct MeshletInstancePrimitiveInfo {
		u32 _materialIndex = 0;
		u32 _vertexOffset = 0;
		u32 _vertexIndexOffset = 0;
		u32 _primitiveOffset = 0;
	};

	struct MeshInstance {
		u32 _stateFlags = MESH_INSTANCE_STATE_FLAGS_NONE;
		u32 _meshIndex = 0;
		u32 _lodMeshInstanceOffset = 0;
		f32 _boundsRadius;
		Float3 _aabbMin;
		Float3 _aabbMax;
		f32 _worldScale = 0.0f;
	};

	struct LodMeshInstance {
		u32 _subMeshInstanceOffset = 0;
		f32 _threshhold = 0.0f;
	};

	struct SubMeshInstance {
		u32 _materialIndex = 0;
		u32 _shaderSetIndex = 0;
	};

	struct DispatchMeshIndirectArgument {
		u32 _meshletInstanceInfoOffset = 0;
		u32 _instanceCount = 0;
		u32 _meshletCount = 0;
		u32 _meshletOffset = 0;
		u32 _dispatchX = 0;
		u32 _dispatchY = 0;
		u32 _dispatchZ = 0;
	};

	struct StarndardMeshIndirectArguments {
		u32 _meshInstanceIndex = 0;
		u32 _materialIndex = 0;
		u32 _indexCountPerInstance = 0;
		u32 _instanceCount = 0;
		u32 _startIndexLocation = 0;
		s32 _baseVertexLocation = 0;
		u32 _startInstanceLocation = 0;
	};

	struct GpuCullingResult {
		u32 _testFrustumCullingSubMeshInstanceCount = 0;
		u32 _passFrustumCullingSubMeshInstanceCount = 0;
		u32 _testOcclusionCullingSubMeshInstanceCount = 0;
		u32 _passOcclusionCullingSubMeshInstanceCount = 0;
		u32 _testOcclusionCullingMeshInstanceCount = 0;
		u32 _passOcclusionCullingMeshInstanceCount = 0;
		u32 _testFrustumCullingMeshInstanceCount = 0;
		u32 _passFrustumCullingMeshInstanceCount = 0;
		u32 _testFrustumCullingMeshletInstanceCount = 0;
		u32 _passFrustumCullingMeshletInstanceCount = 0;
		u32 _testOcclusionCullingMeshletInstanceCount = 0;
		u32 _passOcclusionCullingMeshletInstanceCount = 0;
		u32 _testFrustumCullingTriangleCount = 0;
		u32 _passFrustumCullingTriangleCount = 0;
		u32 _testOcclusionCullingTriangleCount = 0;
		u32 _passOcclusionCullingTriangleCount = 0;
	};

	struct AmplificationCullingResult :public GpuCullingResult {
		u32 _testBackfaceCullingMeshletInstanceCount = 0;
		u32 _passBackfaceCullingMeshletInstanceCount = 0;
		u32 _testBackfaceCullingTriangleCount = 0;
		u32 _passBackfaceCullingTriangleCount = 0;
		u32 _testNdcCullingMeshletInstanceCount = 0;
		u32 _passNdcCullingMeshletInstanceCount = 0;
		u32 _testNdcCullingTriangleCount = 0;
		u32 _passNdcCullingTriangleCount = 0;
	};
}