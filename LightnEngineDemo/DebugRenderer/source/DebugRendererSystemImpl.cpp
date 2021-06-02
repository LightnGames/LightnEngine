#include <DebugRenderer/impl/DebugRendererSystemImpl.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
#include <GfxCore/impl/ViewSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>

DebugRendererSystemImpl _debugSystem;

void DebugRendererSystemImpl::initialize() {
	_lineInstances.initialize(LINE_INSTANCE_CPU_COUNT_MAX);
	_boxInstances.initialize(BOX_INSTANCE_CPU_COUNT_MAX);
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	GraphicsApiInstanceAllocator* apiAllocator = GraphicsApiInstanceAllocator::Get();

	// root signature
	{
		DescriptorRange cbvDescriptorRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		DescriptorRange srvDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		RootParameter rootParameters[2] = {};
		rootParameters[0].initializeDescriptorTable(1, &cbvDescriptorRange, SHADER_VISIBILITY_VERTEX);
		rootParameters[1].initializeDescriptorTable(1, &srvDescriptorRange, SHADER_VISIBILITY_VERTEX);

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = device;
		rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
		rootSignatureDesc._parameters = rootParameters;
		_rootSignature = apiAllocator->allocateRootSignature();
		_rootSignature->iniaitlize(rootSignatureDesc);
	}

	// ms
	{
		ShaderBlob* pixelShader = apiAllocator->allocateShaderBlob();
		pixelShader->initialize("L:/LightnEngine/resource/common/shader/debug/debug3dLine.pso");


		GraphicsPipelineStateDesc defaultPipelineStateDesc = {};
		defaultPipelineStateDesc._device = device;
		defaultPipelineStateDesc._ps = pixelShader->getShaderByteCode();
		defaultPipelineStateDesc._numRenderTarget = 1;
		defaultPipelineStateDesc._rtvFormats[0] = FORMAT_R8G8B8A8_UNORM;
		defaultPipelineStateDesc._topologyType = PRIMITIVE_TOPOLOGY_TYPE_LINE;
		defaultPipelineStateDesc._rootSignature = _rootSignature;
		defaultPipelineStateDesc._depthWriteMask = DEPTH_WRITE_MASK_ZERO;
		defaultPipelineStateDesc._sampleDesc._count = 1;

		// debug line
		{
			_debugLinePipelineState = apiAllocator->allocatePipelineState();
			ShaderBlob* vertexShader = apiAllocator->allocateShaderBlob();
			vertexShader->initialize("L:/LightnEngine/resource/common/shader/debug/debug3dLine.vso");

			GraphicsPipelineStateDesc pipelineStateDesc = defaultPipelineStateDesc;
			pipelineStateDesc._vs = vertexShader->getShaderByteCode();
			_debugLinePipelineState->iniaitlize(pipelineStateDesc);
			vertexShader->terminate();
		}

		// debug box
		{
			_debugBoxPipelineState = apiAllocator->allocatePipelineState();
			ShaderBlob* vertexShader = apiAllocator->allocateShaderBlob();
			vertexShader->initialize("L:/LightnEngine/resource/common/shader/debug/debug3dBox.vso");

			GraphicsPipelineStateDesc pipelineStateDesc = defaultPipelineStateDesc;
			pipelineStateDesc._vs = vertexShader->getShaderByteCode();
			_debugBoxPipelineState->iniaitlize(pipelineStateDesc);
			vertexShader->terminate();
		}

		pixelShader->terminate();
	}

	{
		GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();
		_commandSignature = allocator->allocateCommandSignature();

		IndirectArgumentDesc argumentDescs[1] = {};
		argumentDescs[0]._type = INDIRECT_ARGUMENT_TYPE_DRAW;

		CommandSignatureDesc desc = {};
		desc._device = device;
		desc._byteStride = sizeof(DrawArguments);
		desc._argumentDescs = argumentDescs;
		desc._numArgumentDescs = LTN_COUNTOF(argumentDescs);
		_commandSignature->initialize(desc);
	}

	// buffer
	{
		GpuBufferDesc defaultDesc = {};
		defaultDesc._device = device;
		defaultDesc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		{
			GpuBufferDesc desc = defaultDesc;
			desc._sizeInByte = sizeof(LineInstance) * LINE_INSTANCE_GPU_COUNT_MAX;
			_lineInstanceCpuBuffer.initialize(desc);
			_lineInstanceCpuBuffer.setDebugName("Debug Cpu Line");

			desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			desc._initialState = RESOURCE_STATE_UNORDERED_ACCESS;
			_lineInstanceGpuBuffer.initialize(desc);
			_lineInstanceGpuBuffer.setDebugName("Debug Gpu Line");
		}

		{
			GpuBufferDesc desc = defaultDesc;
			desc._sizeInByte = sizeof(BoxInstance) * BOX_INSTANCE_GPU_COUNT_MAX;
			_boxInstanceCpuBuffer.initialize(desc);
			_boxInstanceCpuBuffer.setDebugName("Debug Cpu Box");

			desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			desc._initialState = RESOURCE_STATE_UNORDERED_ACCESS;
			_boxInstanceGpuBuffer.initialize(desc);
			_boxInstanceGpuBuffer.setDebugName("Debug Gpu Box");
		}
	}

	// indirect argument buffer
	{
		GpuBufferDesc defaultDesc = {};
		defaultDesc._device = device;
		defaultDesc._initialState = RESOURCE_STATE_UNORDERED_ACCESS;
		defaultDesc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		defaultDesc._sizeInByte = sizeof(DrawArguments);
		_lineInstanceIndirectArgumentBuffer.initialize(defaultDesc);
		_boxInstanceIndirectArgumentBuffer.initialize(defaultDesc);
		_lineInstanceIndirectArgumentBuffer.setDebugName("Debug Draw Argument Line");
		_boxInstanceIndirectArgumentBuffer.setDebugName("Debug Draw Argument Box");
	}

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();
	// line instance descriptor
	{
		_lineInstanceCpuSrv = allocator->allocateDescriptors(1);
		_lineInstanceGpuSrv = allocator->allocateDescriptors(1);

		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;
		desc._buffer._numElements = LINE_INSTANCE_GPU_COUNT_MAX;
		desc._buffer._structureByteStride = sizeof(LineInstance);
		device->createShaderResourceView(_lineInstanceCpuBuffer.getResource(), &desc, _lineInstanceCpuSrv._cpuHandle);
		device->createShaderResourceView(_lineInstanceGpuBuffer.getResource(), &desc, _lineInstanceGpuSrv._cpuHandle);
	}

	// box instance descriptor
	{
		_boxInstanceCpuSrv = allocator->allocateDescriptors(1);
		_boxInstanceGpuSrv = allocator->allocateDescriptors(1);

		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;
		desc._buffer._numElements = BOX_INSTANCE_GPU_COUNT_MAX;
		desc._buffer._structureByteStride = sizeof(BoxInstance);
		device->createShaderResourceView(_boxInstanceCpuBuffer.getResource(), &desc, _boxInstanceCpuSrv._cpuHandle);
		device->createShaderResourceView(_boxInstanceGpuBuffer.getResource(), &desc, _boxInstanceGpuSrv._cpuHandle);
	}

	u64 incrimentSize = u64(allocator->getIncrimentSize());
	{
		_lineInstanceGpuUav = allocator->allocateDescriptors(2);
		_boxInstanceGpuUav = allocator->allocateDescriptors(2);
		CpuDescriptorHandle lineInstanceUav = _lineInstanceGpuUav._cpuHandle;
		CpuDescriptorHandle lineInstanceCountUav = _lineInstanceGpuUav._cpuHandle + incrimentSize;
		CpuDescriptorHandle boxInstanceUav = _boxInstanceGpuUav._cpuHandle;
		CpuDescriptorHandle boxInstanceCountUav = _boxInstanceGpuUav._cpuHandle + incrimentSize;

		UnorderedAccessViewDesc defaultDesc = {};
		defaultDesc._format = FORMAT_UNKNOWN;
		defaultDesc._viewDimension = UAV_DIMENSION_BUFFER;
		defaultDesc._buffer._firstElement = 0;
		{
			UnorderedAccessViewDesc desc = defaultDesc;
			desc._buffer._numElements = LINE_INSTANCE_GPU_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(LineInstance);
			device->createUnorderedAccessView(_lineInstanceGpuBuffer.getResource(), nullptr, &desc, lineInstanceUav);
		}

		{
			UnorderedAccessViewDesc desc = defaultDesc;
			desc._buffer._numElements = BOX_INSTANCE_GPU_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(BoxInstance);
			device->createUnorderedAccessView(_boxInstanceGpuBuffer.getResource(), nullptr, &desc, boxInstanceUav);
		}

		{
			UnorderedAccessViewDesc desc = defaultDesc;
			desc._buffer._flags = BUFFER_UAV_FLAG_RAW;
			desc._buffer._numElements = sizeof(DrawArguments) / sizeof(u32);
			desc._format = FORMAT_R32_TYPELESS;
			device->createUnorderedAccessView(_lineInstanceIndirectArgumentBuffer.getResource(), nullptr, &desc, lineInstanceCountUav);
			device->createUnorderedAccessView(_boxInstanceIndirectArgumentBuffer.getResource(), nullptr, &desc, boxInstanceCountUav);
		}
	}
}

void DebugRendererSystemImpl::update() {
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	u32 lineCount = _lineInstances.getCount();
	if (lineCount > 0) {
		LineInstance* lineInstances = vramUpdater->enqueueUpdate<LineInstance>(&_lineInstanceCpuBuffer, 0, lineCount);
		memcpy(lineInstances, _lineInstances.get(), sizeof(LineInstance) * lineCount);
	}

	u32 boxCount = _boxInstances.getCount();
	if (boxCount > 0) {
		BoxInstance* boxInstances = vramUpdater->enqueueUpdate<BoxInstance>(&_boxInstanceCpuBuffer, 0, boxCount);
		memcpy(boxInstances, _boxInstances.get(), sizeof(BoxInstance) * boxCount);
	}

	// debug line
	{
		DrawArguments* arguments = vramUpdater->enqueueUpdate<DrawArguments>(&_lineInstanceIndirectArgumentBuffer, 0);
		arguments->_instanceCount = 0;
		arguments->_startInstanceLocation = 0;
		arguments->_startVertexLocation = 0;
		arguments->_vertexCountPerInstance = 2;
	}

	// debug box
	{
		DrawArguments* arguments = vramUpdater->enqueueUpdate<DrawArguments>(&_boxInstanceIndirectArgumentBuffer, 0);
		arguments->_instanceCount = 0;
		arguments->_startInstanceLocation = 0;
		arguments->_startVertexLocation = 0;
		arguments->_vertexCountPerInstance = 24;
	}
}

void DebugRendererSystemImpl::processDeletion() {
}

void DebugRendererSystemImpl::terminate() {
	processDeletion();
	_lineInstances.terminate();
	_lineInstanceCpuBuffer.terminate();
	_lineInstanceGpuBuffer.terminate();
	_lineInstanceIndirectArgumentBuffer.terminate();
	_boxInstances.terminate();
	_boxInstanceCpuBuffer.terminate();
	_boxInstanceGpuBuffer.terminate();
	_boxInstanceIndirectArgumentBuffer.terminate();
	_debugLinePipelineState->terminate();
	_debugBoxPipelineState->terminate();
	_rootSignature->terminate();
	_commandSignature->terminate();

	DescriptorHeapAllocator* allocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocater->discardDescriptor(_lineInstanceCpuSrv);
	allocater->discardDescriptor(_lineInstanceGpuUav);
	allocater->discardDescriptor(_lineInstanceGpuSrv);
	allocater->discardDescriptor(_boxInstanceCpuSrv);
	allocater->discardDescriptor(_boxInstanceGpuUav);
	allocater->discardDescriptor(_boxInstanceGpuSrv);
}

void DebugRendererSystemImpl::resetGpuCounter(CommandList* commandList) {
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
}

void DebugRendererSystemImpl::render(CommandList* commandList, const ViewInfo* viewInfo) {
	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::GREEN, "Debug Draw");

	DescriptorHandle currentRenderTargetHandle = viewInfo->_hdrRtv;
	commandList->setRenderTargets(1, &currentRenderTargetHandle, nullptr);
	commandList->setViewports(1, &viewInfo->_viewPort);
	commandList->setScissorRects(1, &viewInfo->_scissorRect);

	// cpu line
	{
		u32 instanceCount = _lineInstances.getCount();
		if (instanceCount > 0) {
			commandList->setGraphicsRootSignature(_rootSignature);
			commandList->setPipelineState(_debugLinePipelineState);
			commandList->setPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
			commandList->setGraphicsRootDescriptorTable(0, viewInfo->_viewInfoCbv._gpuHandle);
			commandList->setGraphicsRootDescriptorTable(1, _lineInstanceCpuSrv._gpuHandle);
			commandList->drawInstanced(2, instanceCount, 0, 0);
		}
	}

	// cpu box
	{
		u32 instanceCount = _boxInstances.getCount();
		if (instanceCount > 0) {
			commandList->setGraphicsRootSignature(_rootSignature);
			commandList->setPipelineState(_debugBoxPipelineState);
			commandList->setPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
			commandList->setGraphicsRootDescriptorTable(0, viewInfo->_viewInfoCbv._gpuHandle);
			commandList->setGraphicsRootDescriptorTable(1, _boxInstanceCpuSrv._gpuHandle);
			commandList->drawInstanced(24, instanceCount, 0, 0);
		}
	}

	{
		ResourceTransitionBarrier barriers[4] = {};
		barriers[0] = _lineInstanceIndirectArgumentBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_INDIRECT_ARGUMENT);
		barriers[1] = _boxInstanceIndirectArgumentBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_INDIRECT_ARGUMENT);
		barriers[2] = _lineInstanceGpuBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		barriers[3] = _boxInstanceGpuBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandList->transitionBarriers(barriers, LTN_COUNTOF(barriers));
	}

	// gpu debug line
	{
		commandList->setGraphicsRootSignature(_rootSignature);
		commandList->setPipelineState(_debugLinePipelineState);
		commandList->setPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
		commandList->setGraphicsRootDescriptorTable(0, viewInfo->_viewInfoCbv._gpuHandle);
		commandList->setGraphicsRootDescriptorTable(1, _lineInstanceGpuSrv._gpuHandle);
		commandList->executeIndirect(_commandSignature, 1, _lineInstanceIndirectArgumentBuffer.getResource(), 0, nullptr, 0);
	}

	// gpu debug box
	{
		commandList->setGraphicsRootSignature(_rootSignature);
		commandList->setPipelineState(_debugBoxPipelineState);
		commandList->setPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
		commandList->setGraphicsRootDescriptorTable(0, viewInfo->_viewInfoCbv._gpuHandle);
		commandList->setGraphicsRootDescriptorTable(1, _boxInstanceGpuSrv._gpuHandle);
		commandList->executeIndirect(_commandSignature, 1, _boxInstanceIndirectArgumentBuffer.getResource(), 0, nullptr, 0);
	}

	{
		ResourceTransitionBarrier barriers[4] = {};
		barriers[0] = _lineInstanceIndirectArgumentBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[1] = _boxInstanceIndirectArgumentBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[2] = _lineInstanceGpuBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[3] = _boxInstanceGpuBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->transitionBarriers(barriers, LTN_COUNTOF(barriers));
	}

	_lineInstances.reset();
	_boxInstances.reset();
}

void DebugRendererSystemImpl::drawLine(Vector3 startPosition, Vector3 endPosition, Color4 color) {
	LineInstance* line = _lineInstances.allocate();
	line->_startPosition = startPosition.getFloat3();
	line->_endPosition = endPosition.getFloat3();
	line->_color = color;
}

void DebugRendererSystemImpl::drawBox(Matrix4 matrix, Color4 color) {
	BoxInstance* box = _boxInstances.allocate();
	box->_matrixWorld = matrix.transpose().getMatrix43();
	box->_color = color;
}

void DebugRendererSystemImpl::drawAabb(Vector3 boundsMin, Vector3 boundsMax, Color4 color) {
	Vector3 center = (boundsMin + boundsMax) / 2.0f;
	Vector3 size = boundsMax - boundsMin;
	Matrix4 matrixWorld = Matrix4::scale(size) * Matrix4::translate(center);
	drawBox(matrixWorld, color);
}

void DebugRendererSystemImpl::drawFrustum(Matrix4 view, Matrix4 projection, Color4 color) {
	Vector3 viewTranslate = view.getTranslate();
	Vector3 viewForward(view.m[2][0], view.m[2][1], view.m[2][2]);
	Vector3 viewRight(view.m[0][0], view.m[0][1], view.m[0][2]);
	Vector3 viewUp = Vector3::cross(viewForward, viewRight);

	constexpr f32 VISUALIZE_OFFSET = 0.5f;
	f32 nearClip = -projection.m[3][2] / projection.m[2][2];
	f32 farClip = (nearClip * projection.m[2][2] / (projection.m[2][2] - 1.0f)) * VISUALIZE_OFFSET;
	Vector3 nearProj = viewForward * nearClip;
	Vector3 farProj = viewForward * farClip;
	Vector3 upProj = viewUp * projection.m[0][0];
	Vector3 rightProj = viewRight * (1.0f * projection.m[1][1]);

	Vector3 farCornerRightTop = farProj + rightProj * farClip + upProj * farClip + viewTranslate;
	Vector3 farCornerLeftTop = farProj - rightProj * farClip + upProj * farClip + viewTranslate;
	Vector3 farCornerRightBot = farProj + rightProj * farClip - upProj * farClip + viewTranslate;
	Vector3 farCornerLeftBot = farProj - rightProj * farClip - upProj * farClip + viewTranslate;

	Vector3 nearCornerRightTop = nearProj + rightProj * nearClip + upProj * nearClip + viewTranslate;
	Vector3 nearCornerLeftTop = nearProj - rightProj * nearClip + upProj * nearClip + viewTranslate;
	Vector3 nearCornerRightBot = nearProj + rightProj * nearClip - upProj * nearClip + viewTranslate;
	Vector3 nearCornerLeftBot = nearProj - rightProj * nearClip - upProj * nearClip + viewTranslate;

	drawLine(nearCornerRightTop, farCornerRightTop, color);
	drawLine(nearCornerLeftTop, farCornerLeftTop, color);
	drawLine(nearCornerRightBot, farCornerRightBot, color);
	drawLine(nearCornerLeftBot, farCornerLeftBot, color);

	drawLine(farCornerRightTop, farCornerLeftTop, color);
	drawLine(farCornerLeftTop, farCornerLeftBot, color);
	drawLine(farCornerLeftBot, farCornerRightBot, color);
	drawLine(farCornerRightBot, farCornerRightTop, color);

	drawLine(nearCornerRightTop, nearCornerLeftTop, color);
	drawLine(nearCornerLeftTop, nearCornerLeftBot, color);
	drawLine(nearCornerLeftBot, nearCornerRightBot, color);
	drawLine(nearCornerRightBot, nearCornerRightTop, color);
}

DebugRendererSystem* DebugRendererSystem::Get() {
	return &_debugSystem;
}

DebugRendererSystemImpl* DebugRendererSystemImpl::Get() {
	return &_debugSystem;
}
