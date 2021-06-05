#include <MeshRenderer/impl/MeshRendererSystemImpl.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/ViewSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <MaterialSystem/impl/MaterialSystemImpl.h>
#include <TextureSystem/impl/TextureSystemImpl.h>
#include <DebugRenderer/impl/DebugRendererSystemImpl.h>

MeshRendererSystemImpl _meshRendererSystem;

MeshRendererSystem* MeshRendererSystem::Get() {
	return &_meshRendererSystem;
}

MeshRendererSystemImpl* MeshRendererSystemImpl::Get() {
	return &_meshRendererSystem;
}

#if ENABLE_MESH_SHADER
void MeshRendererSystemImpl::renderMeshShader(CommandList* commandList, ViewInfo* viewInfo) {
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();

	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::RED, "Mesh Shader Pass");

	_gpuCullingResource.resetResultBuffers(commandList);

	// Lod level 計算
	{
		ComputeLodContext context = {};
		context._commandList = commandList;
		context._gpuCullingResource = &_gpuCullingResource;
		context._viewInfo = viewInfo;
		context._meshHandle = _resourceManager.getMeshSrv();
		context._meshInstanceHandle = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		_meshRenderer.computeLod(context);
	}

	// デプスプリパス用　GPUカリング
	{
		GpuCullingContext context = {};
		context._commandList = commandList;
		context._instancingResource = &_instancingResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._cullingViewCbv = viewInfo->_depthPrePassViewInfoCbv._gpuHandle;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._indirectArgumentOffsetSrv = _instancingResource.getInfoOffsetSrv();
		context._subMeshDrawInfoSrv = _resourceManager.getSubMeshDrawInfoSrv();
		context._materialInstanceIndexSrv = _vramShaderSetSystem.getMaterialInstanceIndexSrv()._gpuHandle;
		context._passCulling = _cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESH_CULLING;
		context._scopeName = "Depth Pre Pass Culling";
		_meshRenderer.depthPrePassCulling(context);
	}

	// Build indirect argument
	{
		BuildIndirectArgumentContext context = {};
		context._commandList = commandList;
		context._indirectArgumentResource = &_indirectArgumentResource;
		context._primIndirectArgumentResource = &_primIndirectArgumentResource;
		context._meshletInstanceCountSrv = _instancingResource.getInfoCountSrv();
		context._meshletInstanceOffsetSrv = _instancingResource.getInfoOffsetSrv();
		context._subMeshSrv = _resourceManager.getSubMeshSrv();
		context._buildResource = &_buildIndirectArgumentResource;
		_meshRenderer.buildIndirectArgument(context);
	}

	// デプスプリパス
	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::YELLOW, "Depth Prepass");
		commandList->setViewports(1, &viewInfo->_viewPort);
		commandList->setScissorRects(1, &viewInfo->_scissorRect);

		PipelineStateSet* pipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_AS_MESH_SHADER);
		PipelineStateSet* primPipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_MESH_SHADER);
		RenderContext context = {};
		context._commandList = commandList;
		context._viewInfo = viewInfo;
		context._indirectArgumentResource = &_indirectArgumentResource;
		context._primIndirectArgumentResource = &_primIndirectArgumentResource;
		context._debugFixedViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._vramShaderSets = _vramShaderSetSystem.getShaderSet(0);
		context._meshSrv = _resourceManager.getMeshSrv();
		context._vertexResourceDescriptors = _resourceManager.getVertexSrv();
		context._pipelineStates = pipelineStateSet->_depthPipelineStateGroups;
		context._primInstancingPipelineStates = primPipelineStateSet->_depthPipelineStateGroups;
		context._primCommandSignatures = primPipelineStateSet->_commandSignatures;
		context._commandSignatures = pipelineStateSet->_commandSignatures;
		context._instancingResource = &_instancingResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._scene = &_scene;
		_meshRenderer.render(context);
	}

	// build hiz
	{
		BuildHizContext context = {};
		context._commandList = commandList;
		context._gpuCullingResource = &_gpuCullingResource;
		context._viewInfo = viewInfo;
		_meshRenderer.buildHiz(context);
	}

	// GPUカリング
	{
		GpuCullingContext context = {};
		context._commandList = commandList;
		context._instancingResource = &_instancingResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._cullingViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._indirectArgumentOffsetSrv = _instancingResource.getInfoOffsetSrv();
		context._subMeshDrawInfoSrv = _resourceManager.getSubMeshDrawInfoSrv();
		context._materialInstanceIndexSrv = _vramShaderSetSystem.getMaterialInstanceIndexSrv()._gpuHandle;
		context._passCulling = _cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESH_CULLING;
		context._scopeName = "Main Culling";
		_meshRenderer.mainCulling(context);
	}

	// Build indirect argument
	{
		BuildIndirectArgumentContext context = {};
		context._commandList = commandList;
		context._indirectArgumentResource = &_indirectArgumentResource;
		context._primIndirectArgumentResource = &_primIndirectArgumentResource;
		context._meshletInstanceCountSrv = _instancingResource.getInfoCountSrv();
		context._meshletInstanceOffsetSrv = _instancingResource.getInfoOffsetSrv();
		context._subMeshSrv = _resourceManager.getSubMeshSrv();
		context._buildResource = &_buildIndirectArgumentResource;
		_meshRenderer.buildIndirectArgument(context);
	}

	// 描画
	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Main Pass");
		commandList->setViewports(1, &viewInfo->_viewPort);
		commandList->setScissorRects(1, &viewInfo->_scissorRect);

		PipelineStateSet* pipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_AS_MESH_SHADER);
		PipelineStateSet* primPipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_MESH_SHADER);
		PipelineStateGroup** pipelineStates = getPipelineStateGroup(pipelineStateSet);
		PipelineStateGroup** msPipelineStates = getPipelineStateGroup(primPipelineStateSet);

		if (_cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESHLET_CULLING) {
			pipelineStates = pipelineStateSet->_debugCullingPassPipelineStateGroups;
		}

		LTN_ASSERT(pipelineStates != nullptr);
		_gpuCullingResource.resourceBarriersHizTextureToSrv(commandList);

		RenderContext context = {};
		context._commandList = commandList;
		context._viewInfo = viewInfo;
		context._indirectArgumentResource = &_indirectArgumentResource;
		context._primIndirectArgumentResource = &_primIndirectArgumentResource;
		context._instancingResource = &_instancingResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._debugFixedViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._vramShaderSets = _vramShaderSetSystem.getShaderSet(0);
		context._meshSrv = _resourceManager.getMeshSrv();
		context._vertexResourceDescriptors = _resourceManager.getVertexSrv();
		context._pipelineStates = pipelineStates;
		context._primInstancingPipelineStates = msPipelineStates;
		context._commandSignatures = pipelineStateSet->_commandSignatures;
		context._primCommandSignatures = primPipelineStateSet->_commandSignatures;
		context._scene = &_scene;
		context._collectResult = true;
		_meshRenderer.render(context);

		_gpuCullingResource.resourceBarriersHizSrvToTexture(commandList);
	}

	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Read back Culling Result");
		_gpuCullingResource.readbackCullingResultBuffer(commandList);
	}
}
void MeshRendererSystemImpl::renderMeshShaderDebugFixed(CommandList* commandList, ViewInfo* viewInfo) {
	// GPUカリング
	{
		GpuCullingContext context = {};
		context._commandList = commandList;
		context._instancingResource = &_instancingResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._cullingViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._indirectArgumentOffsetSrv = _instancingResource.getInfoOffsetSrv();
		context._subMeshDrawInfoSrv = _resourceManager.getSubMeshDrawInfoSrv();
		context._materialInstanceIndexSrv = _vramShaderSetSystem.getMaterialInstanceIndexSrv()._gpuHandle;
		context._passCulling = _cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESH_CULLING;
		context._scopeName = "Main Culling";
		_meshRenderer.mainCulling(context);
	}

	// Build indirect argument
	{
		BuildIndirectArgumentContext context = {};
		context._commandList = commandList;
		context._indirectArgumentResource = &_indirectArgumentResource;
		context._primIndirectArgumentResource = &_primIndirectArgumentResource;
		context._meshletInstanceCountSrv = _instancingResource.getInfoCountSrv();
		context._meshletInstanceOffsetSrv = _instancingResource.getInfoOffsetSrv();
		context._subMeshSrv = _resourceManager.getSubMeshSrv();
		context._buildResource = &_buildIndirectArgumentResource;
		_meshRenderer.buildIndirectArgument(context);
	}

	// 描画
	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Main Pass");
		commandList->setViewports(1, &viewInfo->_viewPort);
		commandList->setScissorRects(1, &viewInfo->_scissorRect);

		MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
		PipelineStateSet* pipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_AS_MESH_SHADER);
		PipelineStateSet* primPipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_MESH_SHADER);
		PipelineStateGroup** pipelineStates = getPipelineStateGroup(pipelineStateSet);
		PipelineStateGroup** msPipelineStates = getPipelineStateGroup(primPipelineStateSet);

		if (_cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESHLET_CULLING) {
			pipelineStates = pipelineStateSet->_debugCullingPassPipelineStateGroups;
		}

		LTN_ASSERT(pipelineStates != nullptr);
		_gpuCullingResource.resourceBarriersHizTextureToSrv(commandList);

		RenderContext context = {};
		context._commandList = commandList;
		context._viewInfo = viewInfo;
		context._indirectArgumentResource = &_indirectArgumentResource;
		context._primIndirectArgumentResource = &_primIndirectArgumentResource;
		context._instancingResource = &_instancingResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._debugFixedViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._vramShaderSets = _vramShaderSetSystem.getShaderSet(0);
		context._meshSrv = _resourceManager.getMeshSrv();
		context._vertexResourceDescriptors = _resourceManager.getVertexSrv();
		context._pipelineStates = pipelineStates;
		context._primInstancingPipelineStates = msPipelineStates;
		context._commandSignatures = pipelineStateSet->_commandSignatures;
		context._primCommandSignatures = primPipelineStateSet->_commandSignatures;
		context._scene = &_scene;
		context._collectResult = false;
		_meshRenderer.render(context);

		_gpuCullingResource.resourceBarriersHizSrvToTexture(commandList);
	}
}
#endif

#if ENABLE_MULTI_INDIRECT_DRAW
void MeshRendererSystemImpl::renderMultiIndirect(CommandList* commandList, ViewInfo* viewInfo) {
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::RED, "Multi Draw Pass");

	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	_gpuCullingResource.resetResultBuffers(commandList);

	// Lod level 計算
	{
		ComputeLodContext context = {};
		context._commandList = commandList;
		context._gpuCullingResource = &_gpuCullingResource;
		context._viewInfo = viewInfo;
		context._meshHandle = _resourceManager.getMeshSrv();
		context._meshInstanceHandle = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		_meshRenderer.computeLod(context);
	}

	// デプスプリパス用　GPUカリング
	{
		MultiDrawGpuCullingContext context = {};
		context._commandList = commandList;
		context._indirectArgumentResource = &_multiDrawIndirectArgumentResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._cullingViewCbv = viewInfo->_depthPrePassViewInfoCbv._gpuHandle;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._indirectArgumentOffsetSrv = _multiDrawInstancingResource.getIndirectArgumentOffsetSrv();
		context._subMeshDrawInfoHandle = _resourceManager.getSubMeshDrawInfoSrv();
		context._materialInstanceIndexSrv = _vramShaderSetSystem.getMaterialInstanceIndexSrv()._gpuHandle;
		context._passCulling = _cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESH_CULLING;
		context._scopeName = "Depth Pre Pass Culling";
		_meshRenderer.multiDrawDepthPrePassCulling(context);
	}

	// デプスプリパス
	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::YELLOW, "Depth Pre Pass");

		PipelineStateSet* pipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_CLASSIC);
		MultiIndirectRenderContext context = {};
		context._commandList = commandList;
		context._viewInfo = viewInfo;
		context._indirectArgumentResource = &_multiDrawIndirectArgumentResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._vramShaderSets = _vramShaderSetSystem.getShaderSet(0);
		context._indirectArgmentOffsets = _multiDrawInstancingResource.getIndirectArgumentOffsets();
		context._indirectArgmentCounts = _multiDrawInstancingResource.getIndirectArgumentCounts();
		context._meshInstanceWorldMatrixSrv = _scene.getMeshInstanceWorldMatrixSrv();
		context._pipelineStates = pipelineStateSet->_depthPipelineStateGroups;
		context._commandSignatures = pipelineStateSet->_commandSignatures;
		context._vertexBufferViews = _vertexBufferViews;
		context._indexBufferView = &_indexBufferView;
		context._numVertexBufferView = LTN_COUNTOF(_vertexBufferViews);
		_meshRenderer.multiDrawRender(context);
	}

	// build hiz
	{
		BuildHizContext context = {};
		context._commandList = commandList;
		context._gpuCullingResource = &_gpuCullingResource;
		context._viewInfo = viewInfo;
		_meshRenderer.buildHiz(context);
	}

	// GPUカリング
	{
		MultiDrawGpuCullingContext context = {};
		context._commandList = commandList;
		context._indirectArgumentResource = &_multiDrawIndirectArgumentResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._cullingViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._indirectArgumentOffsetSrv = _multiDrawInstancingResource.getIndirectArgumentOffsetSrv();
		context._subMeshDrawInfoHandle = _resourceManager.getSubMeshDrawInfoSrv();
		context._materialInstanceIndexSrv = _vramShaderSetSystem.getMaterialInstanceIndexSrv()._gpuHandle;
		context._passCulling = _cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESH_CULLING;
		context._scopeName = "Main Culling";
		_meshRenderer.multiDrawMainCulling(context);
	}

	// 描画
	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Main Pass");

		PipelineStateSet* pipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_CLASSIC);
		PipelineStateGroup** pipelineStates = getPipelineStateGroup(pipelineStateSet);

		MultiIndirectRenderContext context = {};
		context._commandList = commandList;
		context._viewInfo = viewInfo;
		context._indirectArgumentResource = &_multiDrawIndirectArgumentResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._vramShaderSets = _vramShaderSetSystem.getShaderSet(0);
		context._indirectArgmentOffsets = _multiDrawInstancingResource.getIndirectArgumentOffsets();
		context._indirectArgmentCounts = _multiDrawInstancingResource.getIndirectArgumentCounts();
		context._meshInstanceWorldMatrixSrv = _scene.getMeshInstanceWorldMatrixSrv();
		context._pipelineStates = pipelineStateSet->_pipelineStateGroups;
		context._commandSignatures = pipelineStateSet->_commandSignatures;
		context._vertexBufferViews = _vertexBufferViews;
		context._indexBufferView = &_indexBufferView;
		context._numVertexBufferView = LTN_COUNTOF(_vertexBufferViews);
		_meshRenderer.multiDrawRender(context);
	}

	_gpuCullingResource.readbackCullingResultBuffer(commandList);
}
void MeshRendererSystemImpl::renderMultiIndirectDebugFixed(CommandList* commandList, ViewInfo* viewInfo) {
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::RED, "Multi Draw Pass");

	// GPUカリング
	{
		MultiDrawGpuCullingContext context = {};
		context._commandList = commandList;
		context._indirectArgumentResource = &_multiDrawIndirectArgumentResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._cullingViewCbv = viewInfo->_cullingViewInfoCbv._gpuHandle;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._indirectArgumentOffsetSrv = _multiDrawInstancingResource.getIndirectArgumentOffsetSrv();
		context._subMeshDrawInfoHandle = _resourceManager.getSubMeshDrawInfoSrv();
		context._materialInstanceIndexSrv = _vramShaderSetSystem.getMaterialInstanceIndexSrv()._gpuHandle;
		context._passCulling = _cullingDebugFlags & CULLING_DEBUG_TYPE_PASS_MESH_CULLING;
		context._scopeName = "Main Culling";
		_meshRenderer.multiDrawMainCulling(context);
	}

	// 描画
	{
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "Main Pass");

		PipelineStateSet* pipelineStateSet = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_CLASSIC);
		PipelineStateGroup** pipelineStates = getPipelineStateGroup(pipelineStateSet);

		MultiIndirectRenderContext context = {};
		context._commandList = commandList;
		context._viewInfo = viewInfo;
		context._indirectArgumentResource = &_multiDrawIndirectArgumentResource;
		context._gpuCullingResource = &_gpuCullingResource;
		context._vramShaderSets = _vramShaderSetSystem.getShaderSet(0);
		context._indirectArgmentOffsets = _multiDrawInstancingResource.getIndirectArgumentOffsets();
		context._indirectArgmentCounts = _multiDrawInstancingResource.getIndirectArgumentCounts();
		context._meshInstanceWorldMatrixSrv = _scene.getMeshInstanceWorldMatrixSrv();
		context._pipelineStates = pipelineStateSet->_pipelineStateGroups;
		context._commandSignatures = pipelineStateSet->_commandSignatures;
		context._vertexBufferViews = _vertexBufferViews;
		context._indexBufferView = &_indexBufferView;
		context._numVertexBufferView = LTN_COUNTOF(_vertexBufferViews);
		_meshRenderer.multiDrawRender(context);
	}
}
#endif

#if ENABLE_CLASSIC_VERTEX
void MeshRendererSystemImpl::renderClassicVertex(CommandList* commandList, ViewInfo* viewInfo) {
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::RED, "Classic Shader Pass");

	GpuBuffer* vertexPositionBuffer = _resourceManager.getPositionVertexBuffer();
	GpuBuffer* vertexTexcoordBuffer = _resourceManager.getTexcoordVertexBuffer();
	GpuBuffer* indexBuffer = _resourceManager.getClassicIndexBuffer();

	ResourceTransitionBarrier toVertexBarriers[4] = {};
	toVertexBarriers[0] = vertexPositionBuffer->getAndUpdateTransitionBarrier(RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	toVertexBarriers[1] = vertexTexcoordBuffer->getAndUpdateTransitionBarrier(RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	toVertexBarriers[2] = indexBuffer->getAndUpdateTransitionBarrier(RESOURCE_STATE_INDEX_BUFFER);
	toVertexBarriers[3] = viewInfo->_depthTexture.getAndUpdateTransitionBarrier(RESOURCE_STATE_DEPTH_WRITE);
	commandList->transitionBarriers(toVertexBarriers, LTN_COUNTOF(toVertexBarriers));

	commandList->setViewports(1, &viewInfo->_viewPort);
	commandList->setScissorRects(1, &viewInfo->_scissorRect);

	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	DescriptorHandle textureDescriptors = TextureSystemImpl::Get()->getDescriptors();
	GpuDescriptorHandle meshInstanceHandle = _scene.getMeshInstanceSrv();
	PipelineStateGroup** pipelineStates = materialSystem->getPipelineStateSet(MaterialSystemImpl::TYPE_CLASSIC)->_pipelineStateGroups;
	u32 meshInstanceCount = _scene.getMeshInstanceCountMax();
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; ++meshInstanceIndex) {
		MeshInstanceImpl* meshInstance = _scene.getMeshInstance(meshInstanceIndex);
		if (!meshInstance->isEnabled()) {
			continue;
		}

		const MeshInfo* meshInfo = meshInstance->getMesh()->getMeshInfo();
		u32 lodLevel = 0;
		u32 lodMeshVertexOffset = meshInstance->getMesh()->getGpuLodMesh(lodLevel)->_vertexOffset;
		u32 lodSubMeshLocalOffset = meshInfo->_subMeshOffsets[lodLevel];
		u32 subMeshCount = meshInstance->getMesh()->getGpuLodMesh()->_subMeshCount;
		for (u32 subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex) {
			const SubMeshInfo* subMeshInfo = meshInstance->getMesh()->getSubMeshInfo(subMeshIndex) + lodSubMeshLocalOffset;
			SubMeshInstance* subMeshInstance = meshInstance->getSubMeshInstance(subMeshIndex) + lodSubMeshLocalOffset;
			Material* material = subMeshInstance->getMaterial();
			ShaderSetImpl* shaderSet = static_cast<MaterialImpl*>(material)->getShaderSet();
			u32 shaderSetIndex = materialSystem->getShaderSetIndex(shaderSet);
			PipelineStateGroup* pipelineState = pipelineStates[shaderSetIndex];
			VramShaderSet* vramShaderSet = _vramShaderSetSystem.getShaderSet(shaderSetIndex);
			commandList->setGraphicsRootSignature(pipelineState->getRootSignature());
			commandList->setPipelineState(pipelineState->getPipelineState());
			commandList->setVertexBuffers(0, LTN_COUNTOF(_vertexBufferViews), _vertexBufferViews);
			commandList->setIndexBuffer(&_indexBufferView);
			commandList->setPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::MATERIALS, vramShaderSet->getMaterialParametersSrv()._gpuHandle);
			commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::SCENE_CONSTANT, viewInfo->_viewInfoCbv._gpuHandle);
			commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::MESH_INSTANCE, meshInstanceHandle);
			commandList->setGraphicsRootDescriptorTable(ClassicMeshRootParam::TEXTURES, textureDescriptors._gpuHandle);

			u32 instanceInfo[2] = {};
			instanceInfo[0] = meshInstanceIndex;
			instanceInfo[1] = meshInstance->getGpuSubMeshInstance(subMeshIndex)->_materialIndex;
			commandList->setGraphicsRoot32BitConstants(ClassicMeshRootParam::MESH_INFO, LTN_COUNTOF(instanceInfo), instanceInfo, 0);

			u32 vertexOffset = lodMeshVertexOffset;
			u32 indexOffset = subMeshInfo->_classiciIndexOffset;
			commandList->drawIndexedInstanced(subMeshInfo->_classicIndexCount, 1, indexOffset, vertexOffset, 0);
		}
	}

	ResourceTransitionBarrier toNonPixelShaderResourceBarriers[4] = {};
	toNonPixelShaderResourceBarriers[0] = vertexPositionBuffer->getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	toNonPixelShaderResourceBarriers[1] = vertexTexcoordBuffer->getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	toNonPixelShaderResourceBarriers[2] = indexBuffer->getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	toNonPixelShaderResourceBarriers[3] = viewInfo->_depthTexture.getAndUpdateTransitionBarrier(RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->transitionBarriers(toNonPixelShaderResourceBarriers, LTN_COUNTOF(toNonPixelShaderResourceBarriers));
}

PipelineStateGroup** MeshRendererSystemImpl::getPipelineStateGroup(PipelineStateSet* pipelineStateSet) {
	switch (_debugPrimitiveType) {
	case DEBUG_PRIMITIVE_TYPE_DEFAULT:
		return pipelineStateSet->_pipelineStateGroups;
	case DEBUG_PRIMITIVE_TYPE_MESHLET:
		return pipelineStateSet->_debugMeshletPipelineStateGroups;
	case DEBUG_PRIMITIVE_TYPE_LODLEVEL:
		return pipelineStateSet->_debugLodLevelPipelineStateGroups;
	case DEBUG_PRIMITIVE_TYPE_OCCLUSION:
		return pipelineStateSet->_debugOcclusionPipelineStateGroups;
	case DEBUG_PRIMITIVE_TYPE_DEPTH:
		return pipelineStateSet->_debugDepthPipelineStateGroups;
	case DEBUG_PRIMITIVE_TYPE_TEXCOORDS:
		return pipelineStateSet->_debugTexcoordsPipelineStateGroups;
	case DEBUG_PRIMITIVE_TYPE_WIREFRAME:
		return pipelineStateSet->_debugWireFramePipelineStateGroups;
	}
	return nullptr;
}

void MeshRendererSystemImpl::setupDraw(CommandList* commandList, ViewInfo* viewInfo) {
	DEBUG_MARKER_GPU_SCOPED_EVENT(commandList, Color4::GREEN, "Setup Draw");

	f32 clearColor[4] = {};
	DescriptorHandle currentRenderTargetHandle = viewInfo->_hdrRtv;
	viewInfo->_depthTexture.transitionResource(commandList, RESOURCE_STATE_DEPTH_WRITE);
	commandList->setRenderTargets(1, &currentRenderTargetHandle, &viewInfo->_depthDsv);
	commandList->clearRenderTargetView(currentRenderTargetHandle, clearColor);
	commandList->clearDepthStencilView(viewInfo->_depthDsv._cpuHandle, CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	viewInfo->_depthTexture.transitionResource(commandList, RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void MeshRendererSystemImpl::debugDrawGpuCullingResult() {
	constexpr char FORMAT1[] = "%7.3f%% ( %-6s/ %-6s)";
	constexpr char FORMAT2[] = "%-12s";
	char t[128];
	const gpu::GpuCullingResult* cullingResult = _gpuCullingResource.getGpuCullingResult();

	if (DebugGui::BeginTabBar("CullingResultTabBar")) {
		if (DebugGui::BeginTabItem("Summary")) {
			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingMeshInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passOcclusionCullingMeshInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassSummaryCullingMeshInstancePersentage(cullingResult);
				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Mesh Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingSubMeshInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passOcclusionCullingSubMeshInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassSummaryCullingSubMeshInstancePersentage(cullingResult);
				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Sub Mesh Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingMeshletInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passOcclusionCullingMeshletInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassSummaryCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingTriangleCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passOcclusionCullingTriangleCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassSummaryCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}
			DebugGui::EndTabItem();
		}
		if (DebugGui::BeginTabItem("Frustum")) {
			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingMeshInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passFrustumCullingMeshInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassFrustumCullingMeshInstancePersentage(cullingResult);
				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Mesh Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingSubMeshInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passFrustumCullingSubMeshInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassFrustumCullingSubMeshInstancePersentage(cullingResult);
				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Sub Mesh Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingMeshletInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passFrustumCullingMeshletInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassFrustumCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingTriangleCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passFrustumCullingTriangleCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassFrustumCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}
		if (DebugGui::BeginTabItem("Occlusion")) {
			{
				ThreeDigiets testOcclusionCullingCount(cullingResult->_testOcclusionCullingMeshInstanceCount);
				ThreeDigiets passOcclusionCullingCount(cullingResult->_passOcclusionCullingMeshInstanceCount);
				f32 passOcclusionCullingPersentage = CullingResult::getPassOcclusionCullingMeshInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passOcclusionCullingPersentage, passOcclusionCullingCount.get(), testOcclusionCullingCount.get());
				DebugGui::ProgressBar(passOcclusionCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Mesh Instance");
			}

			{
				ThreeDigiets testOcclusionCullingCount(cullingResult->_testOcclusionCullingSubMeshInstanceCount);
				ThreeDigiets passOcclusionCullingCount(cullingResult->_passOcclusionCullingSubMeshInstanceCount);
				f32 passOcclusionCullingPersentage = CullingResult::getPassOcclusionCullingSubMeshInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passOcclusionCullingPersentage, passOcclusionCullingCount.get(), testOcclusionCullingCount.get());
				DebugGui::ProgressBar(passOcclusionCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Sub Mesh Instance");
			}

			{
				ThreeDigiets testOcclusionCullingCount(cullingResult->_testOcclusionCullingMeshletInstanceCount);
				ThreeDigiets passOcclusionCullingCount(cullingResult->_passOcclusionCullingMeshletInstanceCount);
				f32 passOcclusionCullingPersentage = CullingResult::getPassOcclusionCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passOcclusionCullingPersentage, passOcclusionCullingCount.get(), testOcclusionCullingCount.get());
				DebugGui::ProgressBar(passOcclusionCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testOcclusionCullingCount(cullingResult->_testOcclusionCullingTriangleCount);
				ThreeDigiets passOcclusionCullingCount(cullingResult->_passOcclusionCullingTriangleCount);
				f32 passOcclusionCullingPersentage = CullingResult::getPassOcclusionCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passOcclusionCullingPersentage, passOcclusionCullingCount.get(), testOcclusionCullingCount.get());
				DebugGui::ProgressBar(passOcclusionCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}
		DebugGui::EndTabBar();
	}
}
void MeshRendererSystemImpl::debugDrawAmplificationCullingResult() {
	constexpr char FORMAT1[] = "%7.3f%% ( %-6s/ %-6s)";
	constexpr char FORMAT2[] = "%-12s";
	char t[128];
	const gpu::AmplificationCullingResult* cullingResult = _gpuCullingResource.getAmplificationCullingResult();

	if (DebugGui::BeginTabBar("CullingResultTabBar")) {
		if (DebugGui::BeginTabItem("Summary")) {
			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingMeshletInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passOcclusionCullingMeshletInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassSummaryCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingTriangleCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passOcclusionCullingTriangleCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassSummaryCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}
		if (DebugGui::BeginTabItem("Frustum")) {
			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingMeshletInstanceCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passFrustumCullingMeshletInstanceCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassFrustumCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testFrustumCullingCount(cullingResult->_testFrustumCullingTriangleCount);
				ThreeDigiets passFrustumCullingCount(cullingResult->_passFrustumCullingTriangleCount);
				f32 passFrustumCullingPersentage = CullingResult::getPassFrustumCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passFrustumCullingPersentage, passFrustumCullingCount.get(), testFrustumCullingCount.get());
				DebugGui::ProgressBar(passFrustumCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}
		if (DebugGui::BeginTabItem("Ndc")) {
			{
				ThreeDigiets testCullingCount(cullingResult->_testNdcCullingMeshletInstanceCount);
				ThreeDigiets passeCullingCount(cullingResult->_passNdcCullingMeshletInstanceCount);
				f32 passeCullingPersentage = CullingResult::getPassNdcCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passeCullingPersentage, passeCullingCount.get(), testCullingCount.get());
				DebugGui::ProgressBar(passeCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testCullingCount(cullingResult->_testNdcCullingTriangleCount);
				ThreeDigiets passCullingCount(cullingResult->_passNdcCullingTriangleCount);
				f32 passCullingPersentage = CullingResult::getPassNdcCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passCullingPersentage, passCullingCount.get(), testCullingCount.get());
				DebugGui::ProgressBar(passCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}

		if (DebugGui::BeginTabItem("Back face")) {
			{
				ThreeDigiets testBackfaceCullingCount(cullingResult->_testBackfaceCullingMeshletInstanceCount);
				ThreeDigiets passBackfaceCullingCount(cullingResult->_passBackfaceCullingMeshletInstanceCount);
				f32 passBackfaceCullingPersentage = CullingResult::getPassBackfaceCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passBackfaceCullingPersentage, passBackfaceCullingCount.get(), testBackfaceCullingCount.get());
				DebugGui::ProgressBar(passBackfaceCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testBackfaceCullingCount(cullingResult->_testBackfaceCullingTriangleCount);
				ThreeDigiets passBackfaceCullingCount(cullingResult->_passBackfaceCullingTriangleCount);
				f32 passBackfaceCullingPersentage = CullingResult::getPassBackfaceCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passBackfaceCullingPersentage, passBackfaceCullingCount.get(), testBackfaceCullingCount.get());
				DebugGui::ProgressBar(passBackfaceCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}

		if (DebugGui::BeginTabItem("Occlusion")) {
			{
				ThreeDigiets testOcclusionCullingCount(cullingResult->_testOcclusionCullingMeshletInstanceCount);
				ThreeDigiets passOcclusionCullingCount(cullingResult->_passOcclusionCullingMeshletInstanceCount);
				f32 passOcclusionCullingPersentage = CullingResult::getPassOcclusionCullingMeshletInstancePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passOcclusionCullingPersentage, passOcclusionCullingCount.get(), testOcclusionCullingCount.get());
				DebugGui::ProgressBar(passOcclusionCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Meshlet Instance");
			}

			{
				ThreeDigiets testOcclusionCullingCount(cullingResult->_testOcclusionCullingTriangleCount);
				ThreeDigiets passOcclusionCullingCount(cullingResult->_passOcclusionCullingTriangleCount);
				f32 passOcclusionCullingPersentage = CullingResult::getPassOcclusionCullingTrianglePersentage(cullingResult);

				sprintf_s(t, LTN_COUNTOF(t), FORMAT1, passOcclusionCullingPersentage, passOcclusionCullingCount.get(), testOcclusionCullingCount.get());
				DebugGui::ProgressBar(passOcclusionCullingPersentage / 100.0f, Vector2(0, 0), t);
				DebugGui::SameLine(0.0f, DebugGui::GetItemInnerSpacing()._x);
				DebugGui::Text(FORMAT2, "Triangle");
			}

			DebugGui::EndTabItem();
		}
		DebugGui::EndTabBar();
	}
}
void MeshRendererSystemImpl::updateVisiblityHighMeshes(s32 visibleType) {
	u32 meshInstanceStepRate = 0;
	switch (visibleType) {
	case 0:
		meshInstanceStepRate = 0xffffffff; // None
		break;
	case 1:
		meshInstanceStepRate = 10; // 1/10
		break;
	case 2:
		meshInstanceStepRate = 1; // All
		break;
	}

	u32 meshInstanceResarveCount = _scene.getMeshInstanceArrayCountMax();
	const u64 bunnyHash = StrHash("victorian\\high\\happy_buddha.mesh");
	const u64 buddhaHash = StrHash("victorian\\high\\bunny.mesh");
	const u64 eratoHash = StrHash("victorian\\high\\erato.mesh");
	const u64 teapotHash = StrHash("victorian\\high\\teapot.mesh");
	const u64 dragonHash = StrHash("victorian\\high\\dragon.mesh");
	MeshInstanceImpl* meshInstances = _scene.getMeshInstance(0);
	const u8* meshInstanceStateFlags = _scene.getMeshInstanceStateFlags();
	u32 counter = 0;
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < meshInstanceResarveCount; ++meshInstanceIndex) {
		if (meshInstanceStateFlags[meshInstanceIndex] != MESH_INSTANCE_FLAG_SCENE_ENABLED) {
			continue;
		}

		MeshInstanceImpl& meshInstance = meshInstances[meshInstanceIndex];
		const DebugMeshInfo* info = meshInstance.getMesh()->getDebugMeshInfo();
		if ((info->_filePathHash == bunnyHash) ||
			(info->_filePathHash == buddhaHash) ||
			(info->_filePathHash == eratoHash) ||
			(info->_filePathHash == teapotHash) ||
			(info->_filePathHash == dragonHash)) {
			u32 cnt = counter++;
			bool visible = (cnt + 1) % meshInstanceStepRate == 0;
			meshInstance.setVisiblity(visible);
		}
	}
}
#endif

void MeshRendererSystemImpl::initialize() {
	_scene.initialize();
	_resourceManager.initialize();
	_meshRenderer.initialize();
	_vramShaderSetSystem.initialize();
	_instancingResource.initialize();

	Device* device = GraphicsSystemImpl::Get()->getDevice();
	GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();

	{
		IndirectArgumentResource::InitializeDesc desc;
		desc._indirectArgumentCount = 1024 * 128;
		desc._indirectArgumentCounterCount = gpu::SHADER_SET_COUNT_MAX;
		desc._strideInByte = sizeof(gpu::DispatchMeshIndirectArgument);
		_indirectArgumentResource.initialize(desc);
	}

	{
		IndirectArgumentResource::InitializeDesc desc;
		desc._indirectArgumentCount = MeshResourceManager::SUB_MESH_COUNT_MAX * gpu::SHADER_SET_COUNT_MAX;
		desc._indirectArgumentCounterCount = gpu::SHADER_SET_COUNT_MAX;
		desc._strideInByte = sizeof(gpu::DispatchMeshIndirectArgument);
		_primIndirectArgumentResource.initialize(desc);
	}

#if ENABLE_MULTI_INDIRECT_DRAW
	_multiDrawInstancingResource.initialize();
	{
		IndirectArgumentResource::InitializeDesc desc;
		desc._indirectArgumentCount = Scene::SUB_MESH_INSTANCE_COUNT_MAX;
		desc._indirectArgumentCounterCount = gpu::SHADER_SET_COUNT_MAX;
		desc._strideInByte = sizeof(gpu::StarndardMeshIndirectArguments);
		_multiDrawIndirectArgumentResource.initialize(desc);
	}
#endif
	_gpuCullingResource.initialize();
	_buildIndirectArgumentResource.initialize();

	// 頂点バッファ・インデックスバッファ初期化
	{
		GpuBuffer* vertexPositionBuffer = _resourceManager.getPositionVertexBuffer();
		GpuBuffer* vertexTexcoordBuffer = _resourceManager.getTexcoordVertexBuffer();
		GpuBuffer* indexBuffer = _resourceManager.getClassicIndexBuffer();

		_vertexBufferViews[0]._bufferLocation = vertexPositionBuffer->getGpuVirtualAddress();
		_vertexBufferViews[0]._sizeInBytes = vertexPositionBuffer->getSizeInByte();
		_vertexBufferViews[0]._strideInBytes = sizeof(Vector3);

		_vertexBufferViews[1]._bufferLocation = vertexTexcoordBuffer->getGpuVirtualAddress();
		_vertexBufferViews[1]._sizeInBytes = vertexTexcoordBuffer->getSizeInByte();
		_vertexBufferViews[1]._strideInBytes = sizeof(u32);

		_indexBufferView._bufferLocation = indexBuffer->getGpuVirtualAddress();
		_indexBufferView._sizeInBytes = indexBuffer->getSizeInByte();
		_indexBufferView._format = FORMAT_R32_UINT;
	}
}

void MeshRendererSystemImpl::terminate() {
	_scene.terminateDefaultResources();
	_resourceManager.terminateDefaultResources();

	processDeletion();

	_scene.terminate();
	_vramShaderSetSystem.terminate();
	_resourceManager.terminate();
	_meshRenderer.terminate();
	_indirectArgumentResource.terminate();
	_primIndirectArgumentResource.terminate();
	_gpuCullingResource.terminate();
	_buildIndirectArgumentResource.terminate();
	_instancingResource.terminate();

#if ENABLE_MULTI_INDIRECT_DRAW
	_multiDrawIndirectArgumentResource.terminate();
	_multiDrawInstancingResource.terminate();
#endif
}

void MeshRendererSystemImpl::update() {
	if (!GraphicsSystemImpl::Get()->isInitialized()) {
		return;
	}

	bool isUpdatedGeometryType = false;
	bool isUpdatedPackMeshletCount = false;

	// メッシュレンダラーデバッグオプション
	{
		struct MeshInstanceDebug {
			bool _visible = true;
			bool _drawMeshInstanceBounds = false;
			bool _drawMeshletBounds = false;
			bool _passMeshInstanceCulling = false;
			bool _passMeshletInstanceCulling = false;
			bool _forceOnlyMeshShader = false;
			s32 _visibleHighPolygonMeshes = 0;
			GeometoryType _geometryMode = GEOMETORY_MODE_MESH_SHADER;
			DebugPrimitiveType _primitiveType = DEBUG_PRIMITIVE_TYPE_DEFAULT;
			s32 _packedMeshletCount = 300;
		};

		auto debug = DebugWindow::StartWindow<MeshInstanceDebug>("Mesh Renderer");
		DebugGui::Checkbox("visible", &debug._visible);
		DebugGui::Checkbox("draw mesh instance bounds", &debug._drawMeshInstanceBounds);
		DebugGui::Checkbox("draw meshlet bounds", &debug._drawMeshletBounds);
		DebugGui::Checkbox("pass mesh culling", &debug._passMeshInstanceCulling);
		DebugGui::Checkbox("pass meshlet culling", &debug._passMeshletInstanceCulling);
		DebugGui::SliderInt("Packed Meshlet", &debug._packedMeshletCount, 0, 350);

		const char* visibleTypes[] = { "None", "Middle", "High" };
		DebugGui::Combo("Visible High Meshes", &debug._visibleHighPolygonMeshes, visibleTypes, LTN_COUNTOF(visibleTypes));

		const char* geometryTypes[] = { "Mesh Shader", "Multi Indirect", "Vertex Shader" };
		DebugGui::Combo("Geometry Type", reinterpret_cast<s32*>(&debug._geometryMode), geometryTypes, LTN_COUNTOF(visibleTypes));

		const char* primitiveTypes[] = { "Default", "Meshlet", "LodLevel", "Occlusion", "Depth", "Texcoords", "Wire Frame" };
		DebugGui::Combo("Primitive Type", reinterpret_cast<s32*>(&debug._primitiveType), primitiveTypes, LTN_COUNTOF(primitiveTypes));
		DebugGui::Checkbox("force mesh shader", &debug._forceOnlyMeshShader);

		if (debug._drawMeshInstanceBounds) {
			_scene.debugDrawMeshInstanceBounds();
		}

		u32 packedMeshletCount = static_cast<u32>(debug._packedMeshletCount);
		if (debug._forceOnlyMeshShader) {
			packedMeshletCount = 0xffff;
		}
		if (_packedMeshletCount != packedMeshletCount) {
			isUpdatedPackMeshletCount = true;
		}

		static s32 visibleHighMeshes = -1;
		if (visibleHighMeshes == -1) {
			debug._visibleHighPolygonMeshes = 2;
			visibleHighMeshes = 2;
		}

		if (visibleHighMeshes != debug._visibleHighPolygonMeshes) {
			updateVisiblityHighMeshes(debug._visibleHighPolygonMeshes);
		}
		visibleHighMeshes = debug._visibleHighPolygonMeshes;

		_packedMeshletCount = packedMeshletCount;
		_debugDrawMeshletBounds = debug._drawMeshletBounds;
		_visible = debug._visible;
		_debugPrimitiveType = debug._primitiveType;

		if (_geometryType != debug._geometryMode) {
			isUpdatedGeometryType = true;
		}
		_geometryType = debug._geometryMode;
		setDebugCullingFlag(CULLING_DEBUG_TYPE_PASS_MESH_CULLING, debug._passMeshInstanceCulling);
		setDebugCullingFlag(CULLING_DEBUG_TYPE_PASS_MESHLET_CULLING, debug._passMeshletInstanceCulling);

		if (DebugGui::BeginTabBar("CullingResultTabBar")) {
			if (DebugGui::BeginTabItem("Gpu Culling")) {
				debugDrawGpuCullingResult();
				DebugGui::EndTabItem();
			}

			if (DebugGui::BeginTabItem("Amplification")) {
				debugDrawAmplificationCullingResult();
				DebugGui::EndTabItem();
			}
			DebugGui::EndTabItem();
		}

		if (DebugGui::BeginTabBar("SceneMeshsTabBar")) {
			if (DebugGui::BeginTabItem("Summary")) {
				// シーン配置メッシュ情報統計
				{
					SceneInfo visibleSceneInfo = _scene.getVisibleSceneInfo();
					SceneInfo sceneInfo = _scene.getSceneInfo();

					const char FORMAT[] = "%25s: %12s/%12s";
					DebugGui::Text(FORMAT, "Scene Info", "Visible", "Total");
					DebugGui::Text(FORMAT, "Mesh Instance Count", ThreeDigiets(visibleSceneInfo._meshInstanceCount), ThreeDigiets(sceneInfo._meshInstanceCount));
					DebugGui::Text(FORMAT, "Lod Mesh Instance Count", ThreeDigiets(visibleSceneInfo._lodMeshInstanceCount), ThreeDigiets(sceneInfo._lodMeshInstanceCount));
					DebugGui::Text(FORMAT, "Sub Mesh Instance Count", ThreeDigiets(visibleSceneInfo._subMeshInstanceCount), ThreeDigiets(sceneInfo._subMeshInstanceCount));
					DebugGui::Text(FORMAT, "Meshlet Instance Count", ThreeDigiets(visibleSceneInfo._meshletInstanceCount), ThreeDigiets(sceneInfo._meshletInstanceCount));
					DebugGui::Text(FORMAT, "Vertex Count", ThreeDigiets(visibleSceneInfo._vertexCount), ThreeDigiets(sceneInfo._vertexCount));
					DebugGui::Text(FORMAT, "Polygon Count", ThreeDigiets(visibleSceneInfo._triangleCount), ThreeDigiets(sceneInfo._triangleCount));
				}
				
				DebugGui::Separator();

				// ユニークリソースメッシュ情報統計
				{
					MeshResourceManagerInfo resourceInfo = _resourceManager.getMeshResourceInfo();

					const char FORMAT[] = "%25s: %12s";
					DebugGui::Text(FORMAT, "Resource Info", "Total");
					DebugGui::Text(FORMAT, "Mesh Count", ThreeDigiets(resourceInfo._meshCount));
					DebugGui::Text(FORMAT, "Lod Mesh Count", ThreeDigiets(resourceInfo._lodMeshCount));
					DebugGui::Text(FORMAT, "Sub Mesh Count", ThreeDigiets(resourceInfo._subMeshCount));
					DebugGui::Text(FORMAT, "Meshlet Count", ThreeDigiets(resourceInfo._meshletCount));
					DebugGui::Text(FORMAT, "Vertex Count", ThreeDigiets(resourceInfo._vertexCount));
					DebugGui::Text(FORMAT, "Polygon Count", ThreeDigiets(resourceInfo._triangleCount));
				}
				DebugGui::EndTabItem();
			}
			if (DebugGui::BeginTabItem("Meshes")) {
				_resourceManager.drawDebugGui();
			}
			if (DebugGui::BeginTabItem("Mesh Instances")) {
				_scene.debugDrawGui();
			}
			if (DebugGui::BeginTabItem("Depth")) {
				ViewInfo* viewInfo = ViewSystemImpl::Get()->getView();
				f32 aspectRate = viewInfo->_viewPort._width / viewInfo->_viewPort._height;
				ResourceDesc desc = viewInfo->_depthTexture.getResourceDesc();
				DebugGui::Image(viewInfo->_depthSrv._gpuHandle, Vector2(200 * aspectRate, 200),
					Vector2(0, 0), Vector2(1, 1), Color4::WHITE, Color4::BLACK, DebugGui::COLOR_CHANNEL_FILTER_R, Vector2(0.95f, 1));
				DebugGui::EndTabItem();
			}
			if (DebugGui::BeginTabItem("Hiz")) {
				_gpuCullingResource.debugDrawHiz();
				DebugGui::EndTabItem();
			}
			DebugGui::EndTabBar();
		}
		DebugWindow::End();
	}

	_scene.update();
	_resourceManager.update();
	_gpuCullingResource.update(ViewSystemImpl::Get()->getView());
	_vramShaderSetSystem.update();

	// Indirect Argument ビルダー更新 
	{
		BuildIndirectArgumentResource::UpdateDesc desc;
		desc._packedMeshletCount = _packedMeshletCount;
		_buildIndirectArgumentResource.update(desc);
	}

	// シーン内のインスタンス・デバッグオプション変更時にインスタンシング用オフセット再計算
	bool updateInstancingOffset = false;
	updateInstancingOffset |= _scene.isUpdatedInstancingOffset();
	updateInstancingOffset |= isUpdatedGeometryType;
	updateInstancingOffset |= isUpdatedPackMeshletCount;
	if (updateInstancingOffset) {
		switch (_geometryType) {
#if ENABLE_MESH_SHADER
		case GEOMETORY_MODE_MESH_SHADER:
		{
			InstancingResource::UpdateDesc desc;
			desc._meshInstances = _scene.getMeshInstance(0);
			desc._countMax = _scene.getMeshInstanceArrayCountMax();
			desc._meshletThresholdUseAmplificationShader = _packedMeshletCount;
			_instancingResource.update(desc);
			break;
		}
#endif
#if ENABLE_MULTI_INDIRECT_DRAW
		case GEOMETORY_MODE_MULTI_INDIRECT:
		{
			MultiDrawInstancingResource::UpdateDesc desc;
			desc._subMeshInstances = _scene.getSubMeshInstances();
			desc._countMax = _scene.getSubMeshInstanceArrayCountMax();
			_multiDrawInstancingResource.update(desc);
			break;
		}
#endif
		}
	}

	// 仮想ビュー用フラスタム範囲デバッグ描画
	if (ViewSystemImpl::Get()->isEnabledDebugFixedView()) {
		const ViewConstantInfo& viewInfo = ViewSystemImpl::Get()->getView()->_cullingViewConstantInfo;
		DebugRendererSystem::Get()->drawFrustum(viewInfo._viewMatrix, viewInfo._projectionMatrix, Color4::YELLOW);
	}
}

void MeshRendererSystemImpl::render(CommandList* commandList, ViewInfo* viewInfo) {
	if (!_visible) {
		return;
	}

	setupDraw(commandList, viewInfo);

	switch (_geometryType) {
#if ENABLE_MESH_SHADER
	case GEOMETORY_MODE_MESH_SHADER:
		renderMeshShader(commandList, viewInfo);
		break;
#endif
#if ENABLE_MULTI_INDIRECT_DRAW
	case GEOMETORY_MODE_MULTI_INDIRECT:
		renderMultiIndirect(commandList, viewInfo);
		break;
#endif
#if ENABLE_CLASSIC_VERTEX
	case GEOMETORY_MODE_CLASSIC_VERTEX:
		renderClassicVertex(commandList, viewInfo);
		break;
#endif
	}

	// メッシュレットバウンディング　デバッグ表示
	if (_debugDrawMeshletBounds) {
		BuildDebugDrawMeshletBoundsContext context;
		context._commandList = commandList;
		context._meshSrv = _resourceManager.getMeshSrv();
		context._meshInstanceSrv = _scene.getMeshInstanceSrv();
		context._meshInstanceWorldMatrixSrv = _scene.getMeshInstanceWorldMatrixSrv();
		context._meshInstanceCountMax = _scene.getMeshInstanceCountMax();
		context._sceneConstantCbv = _scene.getSceneCbv();
		context._currentLodLevelSrv = _gpuCullingResource.getCurrentLodLevelSrv();
		_meshRenderer.buildDebugDrawBounds(context);
	}
}

void MeshRendererSystemImpl::renderDebugFixed(CommandList* commandList, ViewInfo* viewInfo) {
	if (!_visible) {
		return;
	}

	setupDraw(commandList, viewInfo);

	switch (_geometryType) {
#if ENABLE_MESH_SHADER
	case GEOMETORY_MODE_MESH_SHADER:
		renderMeshShaderDebugFixed(commandList, viewInfo);
		break;
#endif
#if ENABLE_MULTI_INDIRECT_DRAW
	case GEOMETORY_MODE_MULTI_INDIRECT:
		renderMultiIndirectDebugFixed(commandList, viewInfo);
		break;
#endif
	}
}

void MeshRendererSystemImpl::processDeletion() {
	_scene.processDeletion();
	_resourceManager.processDeletion();
	_vramShaderSetSystem.processDeletion();
}

Mesh* MeshRendererSystemImpl::allocateMesh(const MeshDesc& desc) {
	return _resourceManager.allocateMesh(desc);
}

Mesh* MeshRendererSystemImpl::createMesh(const MeshDesc& desc) {
	return _resourceManager.createMesh(desc);
}

void MeshRendererSystemImpl::createMeshInstance(MeshInstance** outMeshInstances, const MeshInstanceDesc& desc) {
	return _scene.createMeshInstances(outMeshInstances, desc._meshes, desc._instanceCount);
}

Mesh* MeshRendererSystemImpl::findMesh(u64 filePathHash) {
	return _resourceManager.findMesh(filePathHash);
}
