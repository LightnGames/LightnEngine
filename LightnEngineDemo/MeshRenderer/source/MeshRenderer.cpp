#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <MeshRenderer/GpuStruct.h>
#include <MaterialSystem/impl/PipelineStateSystem.h>
#include <MaterialSystem/impl/MaterialSystemImpl.h>
#include <TextureSystem/impl/TextureSystemImpl.h>

#include <GfxCore/impl/ViewSystemImpl.h>
#include <MeshRenderer/impl/MeshRenderer.h>
#include <MeshRenderer/impl/SceneImpl.h>
#include <MeshRenderer/impl/VramShaderSetSystem.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <DebugRenderer/impl/DebugRendererSystemImpl.h>

void MeshRenderer::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();

	// gpu culling
	{
		DescriptorRange sceneCullingConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		DescriptorRange cullingViewInfoConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		DescriptorRange viewInfoConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
		DescriptorRange meshDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
		DescriptorRange meshInstanceDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 3, 4);
		DescriptorRange indirectArgumentOffsetRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);
		DescriptorRange lodLevelSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 8);
		DescriptorRange subMeshInfoSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);
		DescriptorRange materialInstanceIndexSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);
		DescriptorRange primitiveInstancingInfoOffsetSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 11);
		DescriptorRange hizSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, gpu::HIERACHICAL_DEPTH_COUNT, 13);
		DescriptorRange indirectArgumentUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 2, 0);
		DescriptorRange primitiveInstancingInfoCountUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		DescriptorRange meshletPrimitiveInfoUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		DescriptorRange meshletMeshInstanceIndexUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);
		DescriptorRange cullingResultUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);

		// culling root signature
		{
			_gpuCullingRootSignature = allocator->allocateRootSignature();

			RootParameter rootParameters[GpuCullingRootParam::COUNT] = {};
			rootParameters[GpuCullingRootParam::SCENE_INFO].initializeDescriptorTable(1, &sceneCullingConstantRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::CULLING_VIEW_INFO].initializeDescriptorTable(1, &cullingViewInfoConstantRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::VIEW_INFO].initializeDescriptorTable(1, &viewInfoConstantRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::MESH].initializeDescriptorTable(1, &meshDescriptorRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::MESH_INSTANCE].initializeDescriptorTable(1, &meshInstanceDescriptorRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::INDIRECT_ARGUMENT_OFFSETS].initializeDescriptorTable(1, &indirectArgumentOffsetRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::INDIRECT_ARGUMENTS].initializeDescriptorTable(1, &indirectArgumentUavRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::CULLING_RESULT].initializeDescriptorTable(1, &cullingResultUavRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::LOD_LEVEL].initializeDescriptorTable(1, &lodLevelSrvRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::SUB_MESH_DRAW_INFO].initializeDescriptorTable(1, &subMeshInfoSrvRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::MATERIAL_INSTANCE_INDEX].initializeDescriptorTable(1, &materialInstanceIndexSrvRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::PRIMITIVE_INSTANCING_OFFSETS].initializeDescriptorTable(1, &primitiveInstancingInfoOffsetSrvRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::PRIMITIVE_INSTANCING_COUNTS].initializeDescriptorTable(1, &primitiveInstancingInfoCountUavRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::MESHLET_PRIMITIVE_INFO].initializeDescriptorTable(1, &meshletPrimitiveInfoUavRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::MESHLET_MESH_INSTANCE_INDEX].initializeDescriptorTable(1, &meshletMeshInstanceIndexUavRange, SHADER_VISIBILITY_ALL);
			rootParameters[GpuCullingRootParam::HIZ].initializeDescriptorTable(1, &hizSrvRange, SHADER_VISIBILITY_ALL);

			RootSignatureDesc rootSignatureDesc = {};
			rootSignatureDesc._device = device;
			rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
			rootSignatureDesc._parameters = rootParameters;
			_gpuCullingRootSignature->iniaitlize(rootSignatureDesc);
			_gpuCullingRootSignature->setDebugName("Gpu Culling");
		}

		// pipeline state
		{
			_gpuOcclusionCullingPipelineState = allocator->allocatePipelineState();
			_gpuCullingPipelineState = allocator->allocatePipelineState();

			ComputePipelineStateDesc pipelineStateDesc = {};
			pipelineStateDesc._device = device;
			pipelineStateDesc._rootSignature = _gpuCullingRootSignature;

			// フラスタム　＋　オクルージョンカリング
			{
				ShaderBlob* computeShader = allocator->allocateShaderBlob();
				computeShader->initialize("L:/LightnEngine/resource/common/shader/mesh_shader_gpu_driven/mesh_culling_frustum_occlusion.cso");

				pipelineStateDesc._cs = computeShader->getShaderByteCode();
				_gpuOcclusionCullingPipelineState->iniaitlize(pipelineStateDesc);
				_gpuOcclusionCullingPipelineState->setDebugName("mesh_shader_gpu_driven/mesh_culling_frustum_occlusion.cso");

				computeShader->terminate();
			}

			// フラスタムカリングのみ
			{
				ShaderBlob* computeShader = allocator->allocateShaderBlob();
				computeShader->initialize("L:/LightnEngine/resource/common/shader/mesh_shader_gpu_driven/mesh_culling_frustum.cso");

				pipelineStateDesc._cs = computeShader->getShaderByteCode();
				_gpuCullingPipelineState->iniaitlize(pipelineStateDesc);
				_gpuCullingPipelineState->setDebugName("mesh_shader_gpu_driven/mesh_culling_frustum.cso");

				computeShader->terminate();
			}

			// カリングオフ
			{
				_gpuCullingPassPipelineState = allocator->allocatePipelineState();
				ShaderBlob* computeShader = allocator->allocateShaderBlob();
				computeShader->initialize("L:/LightnEngine/resource/common/shader/mesh_shader_gpu_driven/mesh_culling_pass.cso");
				pipelineStateDesc._cs = computeShader->getShaderByteCode();
				_gpuCullingPassPipelineState->iniaitlize(pipelineStateDesc);
				_gpuCullingPassPipelineState->setDebugName("mesh_shader_gpu_driven/mesh_culling_pass.cso");

				computeShader->terminate();
			}

#if ENABLE_MULTI_INDIRECT_DRAW
			_multiDrawCullingPassPipelineState = allocator->allocatePipelineState();
			{
				ShaderBlob* computeShader = allocator->allocateShaderBlob();
				computeShader->initialize("L:/LightnEngine/resource/common/shader/standard_gpu_driven/mesh_culling_pass.cso");
				pipelineStateDesc._cs = computeShader->getShaderByteCode();
				_multiDrawCullingPassPipelineState->iniaitlize(pipelineStateDesc);
				_multiDrawCullingPassPipelineState->setDebugName("standard_gpu_driven/mesh_culling_pass.cso");

				computeShader->terminate();
			}

			_multiDrawOcclusionCullingPipelineState = allocator->allocatePipelineState();
			{
				ShaderBlob* computeShader = allocator->allocateShaderBlob();
				computeShader->initialize("L:/LightnEngine/resource/common/shader/standard_gpu_driven/mesh_culling_frustum_occlusion.cso");

				pipelineStateDesc._cs = computeShader->getShaderByteCode();
				_multiDrawOcclusionCullingPipelineState->iniaitlize(pipelineStateDesc);
				_multiDrawOcclusionCullingPipelineState->setDebugName("standard_gpu_driven/mesh_culling_frustum_occlusion.cso");

				computeShader->terminate();
			}

			_multiDrawCullingPipelineState = allocator->allocatePipelineState();
			{
				ShaderBlob* computeShader = allocator->allocateShaderBlob();
				computeShader->initialize("L:/LightnEngine/resource/common/shader/standard_gpu_driven/mesh_culling_frustum.cso");
				pipelineStateDesc._cs = computeShader->getShaderByteCode();
				_multiDrawCullingPipelineState->iniaitlize(pipelineStateDesc);
				_multiDrawCullingPipelineState->setDebugName("standard_gpu_driven/mesh_culling_frustum.cso");

				computeShader->terminate();
			}
#endif
		}
	}

	// build indirect arguments
	{
		_buildIndirectArgumentPipelineState = allocator->allocatePipelineState();
		_buildIndirectArgumentRootSignature = allocator->allocateRootSignature();

		DescriptorRange batchedSubMeshInfoOffsetSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		DescriptorRange batchedSubMeshInfoCountSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		DescriptorRange subMeshSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 3, 2);
		DescriptorRange indirectArgumentUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 2, 0);;
		DescriptorRange primIndirectArgumentUavRange(DESCRIPTOR_RANGE_TYPE_UAV, 2, 2);
		DescriptorRange constantCbvRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

		RootParameter rootParameters[BuildIndirectArgumentRootParam::COUNT] = {};
		rootParameters[BuildIndirectArgumentRootParam::BATCHED_SUBMESH_OFFSET].initializeDescriptorTable(1, &batchedSubMeshInfoOffsetSrvRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildIndirectArgumentRootParam::BATCHED_SUBMESH_COUNT].initializeDescriptorTable(1, &batchedSubMeshInfoCountSrvRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildIndirectArgumentRootParam::SUB_MESH].initializeDescriptorTable(1, &subMeshSrvRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildIndirectArgumentRootParam::INDIRECT_ARGUMENT].initializeDescriptorTable(1, &indirectArgumentUavRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildIndirectArgumentRootParam::PRIM_INDIRECT_ARGUMENT].initializeDescriptorTable(1, &primIndirectArgumentUavRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildIndirectArgumentRootParam::CONSTANT].initializeDescriptorTable(1, &constantCbvRange, SHADER_VISIBILITY_ALL);

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = device;
		rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
		rootSignatureDesc._parameters = rootParameters;
		_buildIndirectArgumentRootSignature->iniaitlize(rootSignatureDesc);
		_buildIndirectArgumentRootSignature->setDebugName("Build Indirect Argument");

		ShaderBlob* computeShader = allocator->allocateShaderBlob();
		computeShader->initialize("L:/LightnEngine/resource/common/shader/build_indirect_argument.cso");

		ComputePipelineStateDesc pipelineStateDesc = {};
		pipelineStateDesc._device = device;
		pipelineStateDesc._cs = computeShader->getShaderByteCode();
		pipelineStateDesc._rootSignature = _buildIndirectArgumentRootSignature;
		_buildIndirectArgumentPipelineState->iniaitlize(pipelineStateDesc);
		_buildIndirectArgumentPipelineState->setDebugName("build_indirect_argument.cso");

		computeShader->terminate();
	}

	// gpu compute lod 
	{
		_computeLodPipelineState = allocator->allocatePipelineState();
		_computeLodRootSignature = allocator->allocateRootSignature();

		DescriptorRange sceneCullingConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		DescriptorRange cullingViewInfoConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		DescriptorRange meshDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
		DescriptorRange meshInstanceDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 2, 3);
		DescriptorRange resultLodLevelDescriptorRange(DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		RootParameter rootParameters[GpuComputeLodRootParam::COUNT] = {};
		rootParameters[GpuComputeLodRootParam::SCENE_INFO].initializeDescriptorTable(1, &sceneCullingConstantRange, SHADER_VISIBILITY_ALL);
		rootParameters[GpuComputeLodRootParam::VIEW_INFO].initializeDescriptorTable(1, &cullingViewInfoConstantRange, SHADER_VISIBILITY_ALL);
		rootParameters[GpuComputeLodRootParam::LOD_MESH].initializeDescriptorTable(1, &meshDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[GpuComputeLodRootParam::MESH_INSTANCE].initializeDescriptorTable(1, &meshInstanceDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[GpuComputeLodRootParam::RESULT_LEVEL].initializeDescriptorTable(1, &resultLodLevelDescriptorRange, SHADER_VISIBILITY_ALL);

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = device;
		rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
		rootSignatureDesc._parameters = rootParameters;
		_computeLodRootSignature->iniaitlize(rootSignatureDesc);
		_computeLodRootSignature->setDebugName("Compute Lod");

		ShaderBlob* computeShader = allocator->allocateShaderBlob();
		computeShader->initialize("L:/LightnEngine/resource/common/shader/compute_lod.cso");

		ComputePipelineStateDesc pipelineStateDesc = {};
		pipelineStateDesc._device = device;
		pipelineStateDesc._cs = computeShader->getShaderByteCode();
		pipelineStateDesc._rootSignature = _computeLodRootSignature;
		_computeLodPipelineState->iniaitlize(pipelineStateDesc);
		_computeLodPipelineState->setDebugName("compute_lod.cso");

		computeShader->terminate();
	}

	// debug meshlet bounds
	{
		_debugMeshletBoundsPipelineState = allocator->allocatePipelineState();
		_debugMeshletBoundsRootSignature = allocator->allocateRootSignature();

		DescriptorRange sceneCullingConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		DescriptorRange meshDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
		DescriptorRange meshInstanceDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		DescriptorRange currentLodLevelRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
		DescriptorRange meshInstanceWorldMatrixRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
		DescriptorRange lineDrawIndirectRange(DESCRIPTOR_RANGE_TYPE_UAV, 2, 0);

		RootParameter rootParameters[DebugMeshletBoundsRootParam::COUNT] = {};
		rootParameters[DebugMeshletBoundsRootParam::SCENE_INFO].initializeDescriptorTable(1, &sceneCullingConstantRange, SHADER_VISIBILITY_ALL);
		rootParameters[DebugMeshletBoundsRootParam::MESH].initializeDescriptorTable(1, &meshDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DebugMeshletBoundsRootParam::MESH_INSTANCE].initializeDescriptorTable(1, &meshInstanceDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DebugMeshletBoundsRootParam::LOD_LEVEL].initializeDescriptorTable(1, &currentLodLevelRange, SHADER_VISIBILITY_ALL);
		rootParameters[DebugMeshletBoundsRootParam::MESH_INSTANCE_WORLD_MATRIX].initializeDescriptorTable(1, &meshInstanceWorldMatrixRange, SHADER_VISIBILITY_ALL);
		rootParameters[DebugMeshletBoundsRootParam::INDIRECT_ARGUMENT].initializeDescriptorTable(1, &lineDrawIndirectRange, SHADER_VISIBILITY_ALL);

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = device;
		rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
		rootSignatureDesc._parameters = rootParameters;
		_debugMeshletBoundsRootSignature->iniaitlize(rootSignatureDesc);
		_debugMeshletBoundsRootSignature->setDebugName("Debug Meshlet Bounds");

		ShaderBlob* computeShader = allocator->allocateShaderBlob();
		computeShader->initialize("L:/LightnEngine/resource/common/shader/debug/debug_draw_meshlet_bounds.cso");

		ComputePipelineStateDesc pipelineStateDesc = {};
		pipelineStateDesc._device = device;
		pipelineStateDesc._cs = computeShader->getShaderByteCode();
		pipelineStateDesc._rootSignature = _debugMeshletBoundsRootSignature;
		_debugMeshletBoundsPipelineState->iniaitlize(pipelineStateDesc);
		_debugMeshletBoundsPipelineState->setDebugName("debug/debug_draw_meshlet_bounds.cso");

		computeShader->terminate();
	}

	// build hiz
	{
		_buildHizPipelineState = allocator->allocatePipelineState();
		_buildHizRootSignature = allocator->allocateRootSignature();

		DescriptorRange buildHizInfoConstantRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		DescriptorRange inputDepthRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		DescriptorRange outpuDepthRange(DESCRIPTOR_RANGE_TYPE_UAV, gpu::HIERACHICAL_DEPTH_COUNT / 2, 0);

		RootParameter rootParameters[BuildHizRootParameters::COUNT] = {};
		rootParameters[BuildHizRootParameters::HIZ_INFO].initializeDescriptorTable(1, &buildHizInfoConstantRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildHizRootParameters::INPUT_DEPTH].initializeDescriptorTable(1, &inputDepthRange, SHADER_VISIBILITY_ALL);
		rootParameters[BuildHizRootParameters::OUTPUT_DEPTH].initializeDescriptorTable(1, &outpuDepthRange, SHADER_VISIBILITY_ALL);

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = device;
		rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
		rootSignatureDesc._parameters = rootParameters;
		_buildHizRootSignature->iniaitlize(rootSignatureDesc);
		_buildHizRootSignature->setDebugName("Build Hiz");

		ShaderBlob* computeShader = allocator->allocateShaderBlob();
		computeShader->initialize("L:/LightnEngine/resource/common/shader/build_hierarchical_depth.cso");

		ComputePipelineStateDesc pipelineStateDesc = {};
		pipelineStateDesc._device = device;
		pipelineStateDesc._cs = computeShader->getShaderByteCode();
		pipelineStateDesc._rootSignature = _buildHizRootSignature;
		_buildHizPipelineState->iniaitlize(pipelineStateDesc);
		_buildHizPipelineState->setDebugName("build_hierarchical_depth.cso");

		computeShader->terminate();
	}
}

void MeshRenderer::terminate() {
	_gpuCullingPassPipelineState->terminate();
	_gpuOcclusionCullingPipelineState->terminate();
	_gpuCullingRootSignature->terminate();
	_gpuCullingPipelineState->terminate();
	_computeLodPipelineState->terminate();
	_computeLodRootSignature->terminate();
	_buildHizPipelineState->terminate();
	_buildHizRootSignature->terminate();
	_debugMeshletBoundsPipelineState->terminate();
	_debugMeshletBoundsRootSignature->terminate();
	_buildIndirectArgumentPipelineState->terminate();
	_buildIndirectArgumentRootSignature->terminate();

#if ENABLE_MULTI_INDIRECT_DRAW
	_multiDrawCullingPassPipelineState->terminate();
	_multiDrawCullingPipelineState->terminate();
	_multiDrawOcclusionCullingPipelineState->terminate();
#endif
}

void MeshRenderer::render(const RenderContext& context) const {
	CommandList* commandList = context._commandList;
	ViewInfo* viewInfo = context._viewInfo;
	const Scene* scene = context._scene;
	IndirectArgumentResource* indirectArgumentResource = context._indirectArgumentResource;
	IndirectArgumentResource* primIndirectArgumentResource = context._primIndirectArgumentResource;
	InstancingResource* instancingResource = context._instancingResource;
	QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	DescriptorHandle textureDescriptors = TextureSystemImpl::Get()->getDescriptors();
	const u16* msIndirectArgumentCounts = context._instancingResource->getMsIndirectArgumentCounts();
	const u16* asMsIndirectArgumentCounts = context._instancingResource->getAsMsIndirectArgumentCounts();
	u32 shaderSetCount = materialSystem->getShaderSetCount();
	for (u32 pipelineStateIndex = 0; pipelineStateIndex < shaderSetCount; ++pipelineStateIndex) {
		if (context._pipelineStates[pipelineStateIndex] == nullptr) {
			continue;
		}

		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Shader %d", pipelineStateIndex);
		u32 asMsCommandCountMax = static_cast<u16>(asMsIndirectArgumentCounts[pipelineStateIndex]);
		u32 msCommandCountMax = static_cast<u16>(msIndirectArgumentCounts[pipelineStateIndex]);
		if (asMsCommandCountMax + msCommandCountMax == 0) {
			continue;
		}

		VramShaderSet* vramShaderSet = &context._vramShaderSets[pipelineStateIndex];
		u32 countBufferOffset = pipelineStateIndex * sizeof(u32);
		u32 indirectArgumentOffset = pipelineStateIndex * InstancingResource::INSTANCING_PER_SHADER_COUNT_MAX;
		u32 indirectArgumentOffsetSizeInByte = indirectArgumentOffset * sizeof(gpu::DispatchMeshIndirectArgument);

		// メッシュシェーダー + 増幅シェーダー
		if (asMsCommandCountMax > 0) {
			PipelineStateGroup* pipelineState = context._pipelineStates[pipelineStateIndex];
			commandList->setGraphicsRootSignature(pipelineState->getRootSignature());
			commandList->setPipelineState(pipelineState->getPipelineState());
			setMeshShaderResources(context, vramShaderSet);

			context._gpuCullingResource->setDrawCurrentLodDescriptorTable(commandList);
			if (context._collectResult) {
				context._gpuCullingResource->setDrawResultDescriptorTable(commandList);
			}

			CommandSignature* commandSignature = context._commandSignatures[pipelineStateIndex];
			indirectArgumentResource->executeIndirect(commandList, commandSignature, asMsCommandCountMax, indirectArgumentOffsetSizeInByte, countBufferOffset);
		}

		// メッシュシェーダー
		if (msCommandCountMax > 0) {
			PipelineStateGroup* pipelineState = context._primInstancingPipelineStates[pipelineStateIndex];
			commandList->setGraphicsRootSignature(pipelineState->getRootSignature());
			commandList->setPipelineState(pipelineState->getPipelineState());
			setMeshShaderResources(context, vramShaderSet);

			CommandSignature* commandSignature = context._primCommandSignatures[pipelineStateIndex];
			primIndirectArgumentResource->executeIndirect(commandList, commandSignature, msCommandCountMax, indirectArgumentOffsetSizeInByte, countBufferOffset);
		}
	}
}

void MeshRenderer::computeLod(const ComputeLodContext& context) const {
	CommandList* commandList = context._commandList;
	ViewInfo* viewInfo = context._viewInfo;
	GpuCullingResource* gpuCullingResource = context._gpuCullingResource;
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::BLUE, "Compute Lod");

	commandList->setComputeRootSignature(_computeLodRootSignature);
	commandList->setPipelineState(_computeLodPipelineState);
	gpuCullingResource->setComputeLodResource(commandList);
	gpuCullingResource->resourceBarriersComputeLodToUAV(commandList);

	commandList->setComputeRootDescriptorTable(GpuComputeLodRootParam::SCENE_INFO, context._sceneConstantCbv);
	commandList->setComputeRootDescriptorTable(GpuComputeLodRootParam::VIEW_INFO, viewInfo->_viewInfoCbv._gpuHandle);
	commandList->setComputeRootDescriptorTable(GpuComputeLodRootParam::LOD_MESH, context._meshHandle);
	commandList->setComputeRootDescriptorTable(GpuComputeLodRootParam::MESH_INSTANCE, context._meshInstanceHandle);

	u32 dispatchCount = RoundUp(context._meshInstanceCountMax, 128u);
	commandList->dispatch(dispatchCount, 1, 1);
	gpuCullingResource->resetResourceComputeLodBarriers(commandList);
}

void MeshRenderer::depthPrePassCulling(const GpuCullingContext& context) const {
	PipelineState* pipelineState = context._passCulling ? _gpuCullingPassPipelineState : _gpuCullingPipelineState;
	gpuCulling(context, pipelineState);
}

void MeshRenderer::buildIndirectArgument(const BuildIndirectArgumentContext& context) const {
	CommandList* commandList = context._commandList;
	IndirectArgumentResource* indirectArgumentResource = context._indirectArgumentResource;
	IndirectArgumentResource* primIndirectArgumentResource = context._primIndirectArgumentResource;
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_GREEN, "Build Indirect Argument");

	indirectArgumentResource->resourceBarriersToUav(commandList);
	primIndirectArgumentResource->resourceBarriersToUav(commandList);
	indirectArgumentResource->resetIndirectArgumentCountBuffers(commandList);
	primIndirectArgumentResource->resetIndirectArgumentCountBuffers(commandList);
	commandList->setComputeRootSignature(_buildIndirectArgumentRootSignature);
	commandList->setPipelineState(_buildIndirectArgumentPipelineState);

	commandList->setComputeRootDescriptorTable(BuildIndirectArgumentRootParam::BATCHED_SUBMESH_OFFSET, context._meshletInstanceOffsetSrv);
	commandList->setComputeRootDescriptorTable(BuildIndirectArgumentRootParam::BATCHED_SUBMESH_COUNT, context._meshletInstanceCountSrv);
	commandList->setComputeRootDescriptorTable(BuildIndirectArgumentRootParam::SUB_MESH, context._subMeshSrv);
	commandList->setComputeRootDescriptorTable(BuildIndirectArgumentRootParam::INDIRECT_ARGUMENT, indirectArgumentResource->getIndirectArgumentUav());
	commandList->setComputeRootDescriptorTable(BuildIndirectArgumentRootParam::PRIM_INDIRECT_ARGUMENT, primIndirectArgumentResource->getIndirectArgumentUav());
	commandList->setComputeRootDescriptorTable(BuildIndirectArgumentRootParam::CONSTANT, context._buildResource->getConstantCbv());

	u32 dispatchCount = RoundUp(InstancingResource::INDIRECT_ARGUMENT_COUNTER_COUNT_MAX, 128u);
	commandList->dispatch(dispatchCount, 1, 1);
	indirectArgumentResource->resourceBarriersToIndirectArgument(commandList);
	primIndirectArgumentResource->resourceBarriersToIndirectArgument(commandList);
}

void MeshRenderer::mainCulling(const GpuCullingContext& context) const {
	PipelineState* pipelineState = context._passCulling ? _gpuCullingPassPipelineState : _gpuOcclusionCullingPipelineState;
	gpuCulling(context, pipelineState);
}

void MeshRenderer::buildHiz(const BuildHizContext& context) const {
	CommandList* commandList = context._commandList;
	ViewInfo* viewInfo = context._viewInfo;
	GpuCullingResource* gpuCullingResource = context._gpuCullingResource;
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_BLUE, "Build Hiz");

	viewInfo->_depthTexture.transitionResource(commandList, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->setComputeRootSignature(_buildHizRootSignature);
	commandList->setPipelineState(_buildHizPipelineState);

	// pass 0
	{
		commandList->setComputeRootDescriptorTable(BuildHizRootParameters::INPUT_DEPTH, viewInfo->_depthSrv._gpuHandle);
		gpuCullingResource->resourceBarriersHizTextureToUav(commandList, 0);
		gpuCullingResource->setHizResourcesPass0(commandList);

		u32 dispatchWidthCount = RoundUp(static_cast<u32>(viewInfo->_viewPort._width), 16u);
		u32 dispatchHeightCount = RoundUp(static_cast<u32>(viewInfo->_viewPort._height), 16u);
		commandList->dispatch(dispatchWidthCount, dispatchHeightCount, 1);

		gpuCullingResource->resourceBarriersHizUavtoSrv(commandList, 0);
	}

	// pass 1
	{
		gpuCullingResource->resourceBarriersHizTextureToUav(commandList, 4);
		gpuCullingResource->setHizResourcesPass1(commandList);

		ResourceDesc hizLevel3Desc = gpuCullingResource->getHizTextureResourceDesc(3);
		u32 dispatchWidthCount = RoundUp(static_cast<u32>(hizLevel3Desc._width), 16u);
		u32 dispatchHeightCount = RoundUp(static_cast<u32>(hizLevel3Desc._height), 16u);
		commandList->dispatch(dispatchWidthCount, dispatchHeightCount, 1);

		gpuCullingResource->resourceBarriersHizUavtoSrv(commandList, 4);
	}

	viewInfo->_depthTexture.transitionResource(commandList, RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gpuCullingResource->resourceBarriersHizSrvToTexture(commandList);
}

void MeshRenderer::buildDebugDrawBounds(const BuildDebugDrawMeshletBoundsContext& context) const {
	CommandList* commandList = context._commandList;
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::YELLOW, "Build Debug Meshlet Bounds");

	DebugRendererSystemImpl* debugSystem = DebugRendererSystemImpl::Get();
	commandList->setComputeRootSignature(_debugMeshletBoundsRootSignature);
	commandList->setPipelineState(_debugMeshletBoundsPipelineState);
	commandList->setComputeRootDescriptorTable(DebugMeshletBoundsRootParam::SCENE_INFO, context._sceneConstantCbv);
	commandList->setComputeRootDescriptorTable(DebugMeshletBoundsRootParam::MESH, context._meshSrv);
	commandList->setComputeRootDescriptorTable(DebugMeshletBoundsRootParam::MESH_INSTANCE, context._meshInstanceSrv);
	commandList->setComputeRootDescriptorTable(DebugMeshletBoundsRootParam::MESH_INSTANCE_WORLD_MATRIX, context._meshInstanceWorldMatrixSrv);
	commandList->setComputeRootDescriptorTable(DebugMeshletBoundsRootParam::LOD_LEVEL, context._currentLodLevelSrv);
	commandList->setComputeRootDescriptorTable(DebugMeshletBoundsRootParam::INDIRECT_ARGUMENT, debugSystem->getBoxGpuUavHandle());

	u32 dispatchCount = RoundUp(context._meshInstanceCountMax, 128u);
	commandList->dispatch(dispatchCount, 1, 1);
}

void MeshRenderer::multiDrawRender(const MultiIndirectRenderContext& context) const {
	CommandList* commandList = context._commandList;
	ViewInfo* viewInfo = context._viewInfo;
	GpuCullingResource* gpuCullingResource = context._gpuCullingResource;
	IndirectArgumentResource* indirectArgumentResource = context._indirectArgumentResource;
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	DescriptorHandle textureDescriptors = TextureSystemImpl::Get()->getDescriptors();
	u32 shaderSetCount = materialSystem->getShaderSetCount();

	commandList->setViewports(1, &viewInfo->_viewPort);
	commandList->setScissorRects(1, &viewInfo->_scissorRect);
	viewInfo->_depthTexture.transitionResource(commandList, RESOURCE_STATE_DEPTH_WRITE);
	gpuCullingResource->resourceBarriersHizTextureToSrv(commandList);
	for (u32 pipelineStateIndex = 0; pipelineStateIndex < shaderSetCount; ++pipelineStateIndex) {
		PipelineStateGroup* pipelineState = context._pipelineStates[pipelineStateIndex];
		if (pipelineState == nullptr) {
			continue;
		}

		VramShaderSet* vramShaderSet = &context._vramShaderSets[pipelineStateIndex];
		u32 commandCountMax = context._indirectArgmentCounts[pipelineStateIndex];
		if (commandCountMax == 0) {
			continue;
		}

		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Shader %d", pipelineStateIndex);

		u32 indirectArgumentOffset = context._indirectArgmentOffsets[pipelineStateIndex];
		u32 indirectArgumentOffsetSizeInByte = indirectArgumentOffset * sizeof(gpu::StarndardMeshIndirectArguments);
		commandList->setGraphicsRootSignature(pipelineState->getRootSignature());
		commandList->setPipelineState(pipelineState->getPipelineState());
		commandList->setVertexBuffers(0, context._numVertexBufferView, context._vertexBufferViews);
		commandList->setIndexBuffer(context._indexBufferView);
		commandList->setPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::MATERIALS, vramShaderSet->getMaterialParametersSrv()._gpuHandle);
		commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::SCENE_CONSTANT, viewInfo->_viewInfoCbv._gpuHandle);
		commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::MESH_INSTANCE, context._meshInstanceWorldMatrixSrv);
		commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::TEXTURES, textureDescriptors._gpuHandle);

		u32 countBufferOffset = pipelineStateIndex * sizeof(u32);
		CommandSignature* commandSignature = context._commandSignatures[pipelineStateIndex];
		indirectArgumentResource->executeIndirect(commandList, commandSignature, commandCountMax, indirectArgumentOffsetSizeInByte, countBufferOffset);
	}

	gpuCullingResource->resourceBarriersHizSrvToTexture(commandList);
	viewInfo->_depthTexture.transitionResource(commandList, RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void MeshRenderer::multiDrawDepthPrePassCulling(const MultiDrawGpuCullingContext& context) const {
	PipelineState* pipelineState = context._passCulling ? _multiDrawCullingPassPipelineState : _multiDrawCullingPipelineState;
	gpuCulling(context, pipelineState);
}

void MeshRenderer::multiDrawMainCulling(const MultiDrawGpuCullingContext& context) const {
	PipelineState* pipelineState = context._passCulling ? _multiDrawCullingPassPipelineState : _multiDrawOcclusionCullingPipelineState;
	gpuCulling(context, pipelineState);
}

void MeshRenderer::setMeshShaderResources(const RenderContext& context, VramShaderSet* vramShaderSet) const {
	CommandList* commandList = context._commandList;
	ViewInfo* viewInfo = context._viewInfo;
	const Scene* scene = context._scene;
	InstancingResource* instancingResource = context._instancingResource;
	DescriptorHandle textureDescriptors = TextureSystemImpl::Get()->getDescriptors();

	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::VIEW_CONSTANT, viewInfo->_viewInfoCbv._gpuHandle);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::CULLING_VIEW_CONSTANT, context._debugFixedViewCbv);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MATERIALS, vramShaderSet->getMaterialParametersSrv()._gpuHandle);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MESH, context._meshSrv);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MESHLET_INFO, instancingResource->getPrimitiveInfoSrv());
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MESH_INSTANCE, scene->getMeshInstanceSrv());
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::VERTEX_RESOURCES, context._vertexResourceDescriptors);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::TEXTURES, textureDescriptors._gpuHandle);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MESHLET_PRIMITIVE_INFO, instancingResource->getPrimitiveInfoSrv());
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MESHLET_MESH_INSTANCE_INDEX, instancingResource->getMeshInstanceIndexSrv());
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::MESH_INSTANCE_WORLD_MATRIX, scene->getMeshInstanceWorldMatrixSrv());

	context._gpuCullingResource->setDrawCurrentLodDescriptorTable(commandList);
}

void MeshRenderer::gpuCulling(const GpuCullingContext& context, PipelineState* pipelineState) const {
	u32 meshInstanceCountMax = context._meshInstanceCountMax;
	CommandList* commandList = context._commandList;
	GpuCullingResource* gpuCullingResource = context._gpuCullingResource;
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::GREEN, context._scopeName);

	InstancingResource* instancingResource = context._instancingResource;
	instancingResource->resourceBarriersGpuCullingToUAV(commandList);
	instancingResource->resetInfoCountBuffers(commandList);

	commandList->setComputeRootSignature(_gpuCullingRootSignature);
	commandList->setPipelineState(pipelineState);
	gpuCullingResource->setGpuCullingResources(commandList);

	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::SCENE_INFO, context._sceneConstantCbv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::VIEW_INFO, context._cullingViewCbv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MESH, context._meshSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MESH_INSTANCE, context._meshInstanceSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::SUB_MESH_DRAW_INFO, context._subMeshDrawInfoSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::INDIRECT_ARGUMENT_OFFSETS, context._indirectArgumentOffsetSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MATERIAL_INSTANCE_INDEX, context._materialInstanceIndexSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::PRIMITIVE_INSTANCING_OFFSETS, instancingResource->getInfoOffsetSrv());
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::PRIMITIVE_INSTANCING_COUNTS, instancingResource->getInfoCountUav());
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MESHLET_PRIMITIVE_INFO, instancingResource->getPrimitiveInfoUav());
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MESHLET_MESH_INSTANCE_INDEX, instancingResource->getMeshInstanceIndexUav());

	u32 dispatchCount = RoundUp(meshInstanceCountMax, 128u);
	commandList->dispatch(dispatchCount, 1, 1);
	instancingResource->resetResourceGpuCullingBarriers(commandList);
}

void MeshRenderer::gpuCulling(const MultiDrawGpuCullingContext& context, PipelineState* pipelineState) const {
	u32 meshInstanceCountMax = context._meshInstanceCountMax;
	CommandList* commandList = context._commandList;
	GpuCullingResource* gpuCullingResource = context._gpuCullingResource;
	IndirectArgumentResource* indirectArgumentResource = context._indirectArgumentResource;
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::GREEN, context._scopeName);

	indirectArgumentResource->resourceBarriersToUav(commandList);
	indirectArgumentResource->resetIndirectArgumentCountBuffers(commandList);

	commandList->setComputeRootSignature(_gpuCullingRootSignature);
	commandList->setPipelineState(pipelineState);
	gpuCullingResource->setGpuCullingResources(commandList);

	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::SCENE_INFO, context._sceneConstantCbv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::VIEW_INFO, context._cullingViewCbv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MESH, context._meshSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MESH_INSTANCE, context._meshInstanceSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::SUB_MESH_DRAW_INFO, context._subMeshDrawInfoHandle);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::INDIRECT_ARGUMENT_OFFSETS, context._indirectArgumentOffsetSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::MATERIAL_INSTANCE_INDEX, context._materialInstanceIndexSrv);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::INDIRECT_ARGUMENTS, indirectArgumentResource->getIndirectArgumentUav());

	u32 dispatchCount = RoundUp(meshInstanceCountMax, 128u);
	commandList->dispatch(dispatchCount, 1, 1);
	indirectArgumentResource->resourceBarriersToIndirectArgument(commandList);
}
