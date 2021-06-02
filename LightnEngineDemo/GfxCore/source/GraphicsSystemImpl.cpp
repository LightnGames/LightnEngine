#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/ViewSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <Core/Application.h>

bool _initialized = false;
GraphicsSystemImpl _graphicsSystem;
PipelineState* _pipelineState = nullptr;
RootSignature* _rootSignature = nullptr;

void GraphicsSystemImpl::initialize() {
	GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();
	allocator->initialize();

	HardwareFactoryDesc factoryDesc = {};
	//factoryDesc._flags = HardwareFactoryDesc::FACTROY_FLGA_DEVICE_DEBUG;
	_factory = allocator->allocateHardwareFactroy();
	_factory->initialize(factoryDesc);

	HardwareAdapterDesc adapterDesc = {};
	adapterDesc._factory = _factory;
	_adapter = allocator->allocateHardwareAdapter();
	_adapter->initialize(adapterDesc);

	DeviceDesc deviceDesc = {};
	deviceDesc._adapter = _adapter;
	deviceDesc._debugName = "Device";
	_device = allocator->allocateDevice();
	_device->initialize(deviceDesc);
	LTN_ASSERT(IsSupportedMeshShader(_device));

	CommandQueueDesc commandQueueDesc = {};
	commandQueueDesc._device = _device;
	commandQueueDesc._type = COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc._debugName = "Graphics CommandQueue";
	_commandQueue = allocator->allocateCommandQueue();
	_commandQueue->initialize(commandQueueDesc);

	Application* app = ApplicationSystem::Get()->getApplication();
	u32 screenWidth = app->getScreenWidth();
	u32 screenHeight = app->getScreenHeight();
	SwapChainDesc swapChainDesc = {};
	swapChainDesc._bufferingCount = BACK_BUFFER_COUNT;
	swapChainDesc._format = BACK_BUFFER_FORMAT;
	swapChainDesc._width = screenWidth;
	swapChainDesc._height = screenHeight;
	swapChainDesc._commandQueue = _commandQueue;
	swapChainDesc._hWnd = app->getWindowHandle();
	swapChainDesc._factory = _factory;
	_swapChain = allocator->allocateSwapChain();
	_swapChain->initialize(swapChainDesc);

	DescriptorHeapDesc descriptorAllocatorDesc = {};
	descriptorAllocatorDesc._device = _device;
	descriptorAllocatorDesc._numDescriptors = 16;
	descriptorAllocatorDesc._type = DESCRIPTOR_HEAP_TYPE_RTV;
	_rtvDescriptorAllocator.initialize(descriptorAllocatorDesc);

	descriptorAllocatorDesc._numDescriptors = 16;
	descriptorAllocatorDesc._type = DESCRIPTOR_HEAP_TYPE_DSV;
	_dsvDescriptorAllocator.initialize(descriptorAllocatorDesc);

	// gpu descriptr heap
	descriptorAllocatorDesc._numDescriptors = 1024;
	descriptorAllocatorDesc._flags = DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorAllocatorDesc._type = DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	_srvCbvUavGpuDescriptorAllocator.initialize(descriptorAllocatorDesc);

	// cpu descriptor heap
	descriptorAllocatorDesc._numDescriptors = 128;
	descriptorAllocatorDesc._flags = DESCRIPTOR_HEAP_FLAG_NONE;
	_srvCbvUavCpuDescriptorAllocator.initialize(descriptorAllocatorDesc);

	// スワップチェーンのバックバッファを取得
	u32 incrimentSize = _rtvDescriptorAllocator.getIncrimentSize();
	for (u32 backBufferIndex = 0; backBufferIndex < BACK_BUFFER_COUNT; ++backBufferIndex) {
		char name[128] = {};
		sprintf_s(name, "Render Target %d", backBufferIndex);

		GpuTexture& rtvTexture = _backBuffers[backBufferIndex];
		rtvTexture.initializeFromBackbuffer(_swapChain, backBufferIndex);
		rtvTexture.setDebugName(name);
	}

	{
		ShaderBlob* vertexShader = allocator->allocateShaderBlob();
		ShaderBlob* pixelShader = allocator->allocateShaderBlob();
		vertexShader->initialize("L:/LightnEngine/resource/common/shader/standard_mesh/screen_triangle.vso");
		pixelShader->initialize("L:/LightnEngine/resource/common/shader/standard_mesh/screen_triangle.pso");

		_pipelineState = allocator->allocatePipelineState();
		_rootSignature = allocator->allocateRootSignature();

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = _device;
		_rootSignature->iniaitlize(rootSignatureDesc);

		GraphicsPipelineStateDesc pipelineStateDesc = {};
		pipelineStateDesc._device = _device;
		pipelineStateDesc._vs = vertexShader->getShaderByteCode();
		pipelineStateDesc._ps = pixelShader->getShaderByteCode();
		pipelineStateDesc._numRenderTarget = 1;
		pipelineStateDesc._rtvFormats[0] = FORMAT_R8G8B8A8_UNORM;
		pipelineStateDesc._dsvFormat = FORMAT_D32_FLOAT;
		pipelineStateDesc._topologyType = PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateDesc._rootSignature = _rootSignature;
		pipelineStateDesc._sampleDesc._count = 1;
		_pipelineState->iniaitlize(pipelineStateDesc);

		vertexShader->terminate();
		pixelShader->terminate(); 
	}

	QueryHeapSystem::Get()->initialize();
	ViewSystemImpl::Get()->initialize();

	_vramBufferUpdater.initialize();
	_debugWindow.initialize();

	// タイムスタンプのためにGPU周波数を取得
	QueryHeapSystem::Get()->setGpuFrequency(_commandQueue);
	QueryHeapSystem::Get()->setCpuFrequency();

	_initialized = true;
}

void GraphicsSystemImpl::update() {
	GraphicsApiInstanceAllocator::Get()->update();
	ViewSystemImpl::Get()->update();
	_vramBufferUpdater.update();

	QueryHeapSystem::Get()->debugDrawTimeStamps();
	QueryHeapSystem::Get()->update();
}

void GraphicsSystemImpl::processDeletion() {
}

void GraphicsSystemImpl::moveToNextFrame() {
	u64 currentFenceValue = _fenceValues[_frameIndex];
	_commandQueue->waitForFence(currentFenceValue);

	_frameIndex = _swapChain->getCurrentBackBufferIndex();

	u64 fenceValue = _commandQueue->incrimentFence();
	_fenceValues[_frameIndex] = fenceValue;
}

void GraphicsSystemImpl::render() {
	CommandListDesc commandListDesc = {};
	commandListDesc._device = _device;
	commandListDesc._type = COMMAND_LIST_TYPE_DIRECT;

	char name[128] = {};
	sprintf_s(name, "CommandList %d", _frameIndex);
	commandListDesc._debugName = name;

	QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
	u64 compleatedFenceValue = _commandQueue->getCompleatedValue();
	GraphicsApiInstanceAllocator* allocater = GraphicsApiInstanceAllocator::Get();
	CommandList* commandList = allocater->allocateCommandList(compleatedFenceValue);
	commandList->initialize(commandListDesc);

	{
		// 描画開始時の時間計測マーカーを追加
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_RED, "End Frame");

		DescriptorHeap* descriptorHeaps[] = { _srvCbvUavGpuDescriptorAllocator.getDescriptorHeap() };
		commandList->setDescriptorHeaps(LTN_COUNTOF(descriptorHeaps), descriptorHeaps);
		_vramBufferUpdater.populateCommandList(commandList);

		_renderPass(commandList);
		_debugWindow.renderFrame(commandList);

		// hdr buffer からバックバッファにコピー
		{
			DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::GREEN, "HDR to LDR");

			ViewInfo* viewInfo = ViewSystemImpl::Get()->getView();
			GpuTexture& currentRtvTexture = _backBuffers[_frameIndex];
			ResourceTransitionBarrier barriers[2] = {};
			barriers[0] = viewInfo->_hdrTexture.getAndUpdateTransitionBarrier(RESOURCE_STATE_COPY_SOURCE);
			barriers[1] = currentRtvTexture.getAndUpdateTransitionBarrier(RESOURCE_STATE_COPY_DEST);

			ResourceTransitionBarrier resetBarriers[2] = {};
			resetBarriers[0] = viewInfo->_hdrTexture.getAndUpdateTransitionBarrier(RESOURCE_STATE_RENDER_TARGET);
			resetBarriers[1] = currentRtvTexture.getAndUpdateTransitionBarrier(RESOURCE_STATE_PRESENT);

			commandList->transitionBarriers(barriers, LTN_COUNTOF(barriers));
			commandList->copyResource(currentRtvTexture.getResource(), viewInfo->_hdrTexture.getResource());
			commandList->transitionBarriers(resetBarriers, LTN_COUNTOF(resetBarriers));
		}
	}


	// タイムスタンプ更新
	queryHeapSystem->requestTimeStamp(commandList, _frameIndex);

	_commandQueue->executeCommandLists(1, &commandList);

	_swapChain->present(1, 0);
	moveToNextFrame();
}

void GraphicsSystemImpl::terminate() {
	processDeletion();
	_debugWindow.terminate();
	_vramBufferUpdater.terminate();

	_rootSignature->terminate();
	_pipelineState->terminate();

	QueryHeapSystem::Get()->terminate();
	ViewSystemImpl::Get()->terminate();

	_commandQueue->waitForIdle();
	for (u32 backBufferIndex = 0; backBufferIndex < BACK_BUFFER_COUNT; ++backBufferIndex) {
		_backBuffers[backBufferIndex].terminate();
	}

	_dsvDescriptorAllocator.terminate();
	_rtvDescriptorAllocator.terminate();
	_srvCbvUavGpuDescriptorAllocator.terminate();
	_srvCbvUavCpuDescriptorAllocator.terminate();
	_commandQueue->terminate();
	_swapChain->terminate();
	_factory->terminate();
	_adapter->terminate();
	_device->terminate();

	GraphicsApiInstanceAllocator::Get()->update();
	GraphicsApiInstanceAllocator::Get()->terminate();
}

GraphicsSystem* GraphicsSystem::Get() {
	return &_graphicsSystem;
}

bool GraphicsSystemImpl::isInitialized() const {
	return _initialized;
}

void GraphicsSystemImpl::beginDebugWindow() {
	_debugWindow.beginFrame();
}

GraphicsSystemImpl* GraphicsSystemImpl::Get() {
	return &_graphicsSystem;
}
