#include "GraphicsApiInterfaceImpl.h"
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <D3Dcompiler.h>
#include "../third_party/Imgui/imgui.h"
#include "../third_party/Imgui/imgui_impl_win32.h"
#include "../third_party/Imgui/imgui_impl_dx12.h"
#include "../third_party/pix/pix3.h"

GraphicsApiInstanceAllocatorImpl _graphicsApiInstanceAllocator;

D3D12_FILL_MODE toD3d12(FillMode mode) {
	return static_cast<D3D12_FILL_MODE>(mode);
}

D3D12_COMPARISON_FUNC toD3d12(ComparisonFunc func) {
	return static_cast<D3D12_COMPARISON_FUNC>(func);
}

D3D12_DEPTH_WRITE_MASK toD3d12(DepthWriteMask mask) {
	return static_cast<D3D12_DEPTH_WRITE_MASK>(mask);
}

ID3D12DescriptorHeap* toD3d12(DescriptorHeap* descriptorHeap) {
	return static_cast<DescriptorHeapD3D12*>(descriptorHeap)->_descriptorHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE toD3d12(CpuDescriptorHandle handle) {
	D3D12_CPU_DESCRIPTOR_HANDLE result = { handle._ptr };
	return result;
}

D3D12_GPU_DESCRIPTOR_HANDLE toD3d12(GpuDescriptorHandle handle) {
	D3D12_GPU_DESCRIPTOR_HANDLE result = { handle._ptr };
	return result;
}

ID3D12GraphicsCommandList* toD3d12(CommandList* commandList) {
	return static_cast<CommandListD3D12*>(commandList)->_commandList;
}

DXGI_FORMAT toD3d12(Format format) {
	return static_cast<DXGI_FORMAT>(format);
}

ID3D12RootSignature* toD3d12(RootSignature* rootSignature) {
	return static_cast<RootSignatureD3D12*>(rootSignature)->_rootSignature;
}

ID3D12Resource* toD3d12(Resource* resource) {
	if (resource == nullptr) {
		return nullptr;
	}
	return static_cast<ResourceD3D12*>(resource)->_resource;
}

D3D12_SHADER_BYTECODE toD3d12(ShaderByteCode byteCode) {
	D3D12_SHADER_BYTECODE result = {};
	result.BytecodeLength = byteCode._bytecodeLength;
	result.pShaderBytecode = byteCode._shaderBytecode;
	return result;
}

ID3D12Device2* toD3d12(Device* device) {
	return static_cast<DeviceD3D12*>(device)->_device;
}

D3D12_HEAP_TYPE toD3d12(HeapType type) {
	return static_cast<D3D12_HEAP_TYPE>(type);
}

D3D12_HEAP_FLAGS toD3d12(HeapFlags flags) {
	return static_cast<D3D12_HEAP_FLAGS>(flags);
}

D3D12_RESOURCE_STATES toD3d12(ResourceStates states) {
	return static_cast<D3D12_RESOURCE_STATES>(states);
}

const D3D12_CLEAR_VALUE* toD3d12(const ClearValue* value) {
	return reinterpret_cast<const D3D12_CLEAR_VALUE*>(value);
}

D3D12_CONSTANT_BUFFER_VIEW_DESC toD3d12(ConstantBufferViewDesc desc) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC result = {};
	result.BufferLocation = desc._bufferLocation;
	result.SizeInBytes = desc._sizeInBytes;
	return result;
}

D3D12_RESOURCE_DESC toD3d12(ResourceDesc desc) {
	D3D12_RESOURCE_DESC result = {};
	memcpy(&result, &desc, sizeof(result));
	return result;
}

const D3D12_INPUT_ELEMENT_DESC* toD3d12(const InputElementDesc* desc) {
	return reinterpret_cast<const D3D12_INPUT_ELEMENT_DESC*>(desc);
}

D3D12_COMMAND_SIGNATURE_DESC toD3d12(CommandSignatureDesc desc) {
	D3D12_COMMAND_SIGNATURE_DESC result = {};
	result.ByteStride = desc._byteStride;
	result.NodeMask = desc._nodeMask;
	result.NumArgumentDescs = desc._numArgumentDescs;
	result.pArgumentDescs = reinterpret_cast<const D3D12_INDIRECT_ARGUMENT_DESC*>(desc._argumentDescs);
	return result;
}

ID3D12CommandSignature* toD3d12(CommandSignature* commandSignature) {
	return static_cast<CommandSignatureD3D12*>(commandSignature)->_commandSignature;
}

D3D12_PIPELINE_STATE_FLAGS toD3d12(PipelineStateFlags flags) {
	return static_cast<D3D12_PIPELINE_STATE_FLAGS>(flags);
}

const D3D12_RECT* toD3d12(const Rect* rect) {
	return reinterpret_cast<const D3D12_RECT*>(rect);
}

D3D12_CLEAR_FLAGS toD3d12(ClearFlags flags) {
	return static_cast<D3D12_CLEAR_FLAGS>(flags);
}

const D3D12_BOX* toD3d12(const Box* box) {
	return reinterpret_cast<const D3D12_BOX*>(box);
}

D3D12_PLACED_SUBRESOURCE_FOOTPRINT toD3d12(PlacedSubresourceFootprint layout) {
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT result = {};
	result.Offset = layout._offset;
	result.Footprint.Depth = layout._footprint._depth;
	result.Footprint.Format = toD3d12(layout._footprint._format);
	result.Footprint.Height = layout._footprint._height;
	result.Footprint.RowPitch = layout._footprint._rowPitch;
	result.Footprint.Width = layout._footprint._width;
	return result;
}

D3D12_TEXTURE_COPY_LOCATION toD3d12(TextureCopyLocation location) {
	D3D12_TEXTURE_COPY_LOCATION result = {};
	result.pResource = toD3d12(location._resource);
	result.Type = static_cast<D3D12_TEXTURE_COPY_TYPE>(location._type);
	result.PlacedFootprint = toD3d12(location._placedFootprint);
	return result;
}

const D3D12_RESOURCE_DESC* toD3d12(const ResourceDesc* desc) {
	return reinterpret_cast<const D3D12_RESOURCE_DESC*>(desc);
}

const D3D12_VERTEX_BUFFER_VIEW* toD3d12(const VertexBufferView* view) {
	return reinterpret_cast<const D3D12_VERTEX_BUFFER_VIEW*>(view);
}

const D3D12_INDEX_BUFFER_VIEW* toD3d12(const IndexBufferView* view) {
	return reinterpret_cast<const D3D12_INDEX_BUFFER_VIEW*>(view);
}

D3D12_BLEND toD3d12(const Blend& blend) {
	return static_cast<D3D12_BLEND>(blend);
}

D3D12_BLEND_OP toD3d12(const BlendOp& blend) {
	return static_cast<D3D12_BLEND_OP>(blend);
}

D3D12_LOGIC_OP toD3d12(const LogicOp& blend) {
	return static_cast<D3D12_LOGIC_OP>(blend);
}

ID3D12QueryHeap* toD3d12(QueryHeap* queryHeap) {
	return static_cast<QueryHeapD3D12*>(queryHeap)->_queryHeap;
}

D3D12_RENDER_TARGET_BLEND_DESC toD3d12(const RenderTargetBlendDesc& desc) {
	D3D12_RENDER_TARGET_BLEND_DESC result = {};
	result.BlendEnable = desc._blendEnable;
	result.LogicOpEnable = desc._logicOpEnable;
	result.SrcBlend = toD3d12(desc._srcBlend);
	result.DestBlend = toD3d12(desc._destBlend);
	result.BlendOp = toD3d12(desc._blendOp);
	result.SrcBlendAlpha = toD3d12(desc._srcBlendAlpha);
	result.DestBlendAlpha = toD3d12(desc._destBlendAlpha);
	result.BlendOpAlpha = toD3d12(desc._blendOpAlpha);
	result.LogicOp = toD3d12(desc._logicOp);
	result.RenderTargetWriteMask = desc._renderTargetWriteMask;
	return result;
}

D3D12_BLEND_DESC toD3d12(const BlendDesc& desc) {
	D3D12_BLEND_DESC result = {};
	result.AlphaToCoverageEnable = desc._alphaToCoverageEnable;
	result.IndependentBlendEnable = desc._independentBlendEnable;
	for (u32 i = 0; i < LTN_COUNTOF(desc._renderTarget); ++i) {
		result.RenderTarget[i] = toD3d12(desc._renderTarget[i]);
	}
	return result;
}

template<class T>
void SetDebugName(T* resource, const char* name, ...) {
	constexpr u32 SET_NAME_LENGTH_COUNT_MAX = 128;
	char nameBuffer[SET_NAME_LENGTH_COUNT_MAX] = {};
	va_list va;
	va_start(va, name);
	vsprintf_s(nameBuffer, name, va);
	va_end(va);

	WCHAR wName[SET_NAME_LENGTH_COUNT_MAX] = {};
	size_t wLength = 0;
	mbstowcs_s(&wLength, wName, SET_NAME_LENGTH_COUNT_MAX, nameBuffer, _TRUNCATE);
	resource->SetName(wName);
}

void HardwareFactoryD3D12::initialize(const HardwareFactoryDesc& desc) {
#define ENABLE_GBV 0
	u32 dxgiFactoryFlags = 0;

	// debug Layer
	if (desc._flags & HardwareFactoryDesc::FACTROY_FLGA_DEVICE_DEBUG) {
		ID3D12Debug* debugController = nullptr;
#if ENABLE_GBV
		ID3D12Debug1* debugController1 = nullptr;
#endif
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

#if ENABLE_GBV
			debugController->QueryInterface(IID_PPV_ARGS(&debugController1));
			debugController1->SetEnableGPUBasedValidation(true);
#endif
		}

		debugController->Release();
#if ENABLE_GBV
		debugController1->Release();
#endif
	}

	LTN_SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&_factory)));
}

void HardwareFactoryD3D12::terminate() {
	_factory->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

void HardwareAdapterD3D12::initialize(const HardwareAdapterDesc& desc) {
	IDXGIFactory4* factory = static_cast<HardwareFactoryD3D12*>(desc._factory)->_factory;
	for (u32 adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters(adapterIndex, reinterpret_cast<IDXGIAdapter**>(&_adapter)); ++adapterIndex) {
		DXGI_ADAPTER_DESC1 desc;
		_adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(_adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			break;
		}
	}
}

void HardwareAdapterD3D12::terminate() {
	_adapter->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

QueryVideoMemoryInfo HardwareAdapterD3D12::queryVideoMemoryInfo() {
	DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
	_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);

	QueryVideoMemoryInfo info;
	info._budget = videoMemoryInfo.Budget;
	info._availableForReservation = videoMemoryInfo.AvailableForReservation;
	info._currentReservation = videoMemoryInfo.CurrentReservation;
	info._currentUsage = videoMemoryInfo.CurrentUsage;
	return info;
}

void DeviceD3D12::initialize(const DeviceDesc& desc) {
	IDXGIAdapter1* adapter = static_cast<HardwareAdapterD3D12*>(desc._adapter)->_adapter;
	LTN_SUCCEEDED(D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&_device)
	));
	SetDebugName(_device, desc._debugName);

	// エラー発生時にブレーク
	ID3D12InfoQueue* infoQueue;
	_device->QueryInterface(IID_PPV_ARGS(&infoQueue));

	if (infoQueue != nullptr) {
		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
		};
		D3D12_MESSAGE_SEVERITY severities[] = {
		  D3D12_MESSAGE_SEVERITY_INFO
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);
		infoQueue->Release();
	}
}

void DeviceD3D12::terminate() {
	_device->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

u32 DeviceD3D12::getDescriptorHandleIncrementSize(DescriptorHeapType type) const {
	return _device->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type));
}

void DeviceD3D12::createRenderTargetView(Resource* resource, CpuDescriptorHandle destDescriptor) {
	_device->CreateRenderTargetView(toD3d12(resource), nullptr, toD3d12(destDescriptor));
}

void DeviceD3D12::createDepthStencilView(Resource* resource, CpuDescriptorHandle destDescriptor) {
	_device->CreateDepthStencilView(toD3d12(resource), nullptr, toD3d12(destDescriptor));
}

void DeviceD3D12::createCommittedResource(HeapType heapType, HeapFlags heapFlags, const ResourceDesc& desc,
	ResourceStates initialResourceState, const ClearValue* optimizedClearValue, Resource* dstResource) {
	ID3D12Resource* resource = nullptr;
	D3D12_RESOURCE_DESC resourceDesc = toD3d12(desc);
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = toD3d12(heapType);

	LTN_SUCCEEDED(_device->CreateCommittedResource(&heapProperties, toD3d12(heapFlags), &resourceDesc, toD3d12(initialResourceState), toD3d12(optimizedClearValue), IID_PPV_ARGS(&resource)));
	static_cast<ResourceD3D12*>(dstResource)->initialize(resource);
}

void DeviceD3D12::createConstantBufferView(const ConstantBufferViewDesc& desc, CpuDescriptorHandle destDescriptor) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC descD3d12 = toD3d12(desc);
	_device->CreateConstantBufferView(&descD3d12, toD3d12(destDescriptor));
}

void DeviceD3D12::createShaderResourceView(Resource* resource, const ShaderResourceViewDesc* desc, CpuDescriptorHandle destDescriptor) {
	const D3D12_SHADER_RESOURCE_VIEW_DESC* descD3d12 = nullptr;
	if (desc != nullptr) {
		descD3d12 = reinterpret_cast<const D3D12_SHADER_RESOURCE_VIEW_DESC*>(desc);
	}
	_device->CreateShaderResourceView(toD3d12(resource), descD3d12, toD3d12(destDescriptor));
}

void DeviceD3D12::createUnorderedAccessView(Resource* resource, Resource* counterResource, const UnorderedAccessViewDesc* desc, CpuDescriptorHandle destDescriptor) {
	const D3D12_UNORDERED_ACCESS_VIEW_DESC* descD3d12 = nullptr;
	ID3D12Resource* counterResourceD3d12 = nullptr;
	if (desc != nullptr) {
		descD3d12 = reinterpret_cast<const D3D12_UNORDERED_ACCESS_VIEW_DESC*>(desc);
	}

	if (counterResource != nullptr) {
		counterResourceD3d12 = toD3d12(counterResource);
	}
	_device->CreateUnorderedAccessView(toD3d12(resource), counterResourceD3d12, descD3d12, toD3d12(destDescriptor));
}

void DeviceD3D12::getCopyableFootprints(const ResourceDesc* resourceDesc, u32 firstSubresource, u32 numSubresources, u64 baseOffset, PlacedSubresourceFootprint* layouts, u32* numRows, u64* rowSizeInBytes, u64* totalBytes) {
	_device->GetCopyableFootprints(toD3d12(resourceDesc), firstSubresource, numSubresources, baseOffset, reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(layouts), numRows, rowSizeInBytes, totalBytes);
}

u8 DeviceD3D12::getFormatPlaneCount(Format format) {
	D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = { toD3d12(format), 0 };
	if (FAILED(_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo)))) {
		return 0;
	}
	return formatInfo.PlaneCount;
}

void SwapChainD3D12::initialize(const SwapChainDesc& desc) {
	IDXGIFactory4* factory = static_cast<HardwareFactoryD3D12*>(desc._factory)->_factory;
	ID3D12CommandQueue* commandQueue = static_cast<CommandQueueD3D12*>(desc._commandQueue)->_commandQueue;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = desc._bufferingCount;
	swapChainDesc.Width = desc._width;
	swapChainDesc.Height = desc._height;
	swapChainDesc.Format = toD3d12(desc._format);
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	HWND hwnd = reinterpret_cast<HWND>(desc._hWnd);
	IDXGISwapChain1* swapChain = nullptr;
	LTN_SUCCEEDED(factory->CreateSwapChainForHwnd(
		commandQueue,
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	LTN_SUCCEEDED(swapChain->QueryInterface(__uuidof(IDXGISwapChain3), reinterpret_cast<void**>(&_swapChain)));

	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	swapChain->Release();
}

void SwapChainD3D12::terminate() {
	_swapChain->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

u32 SwapChainD3D12::getCurrentBackBufferIndex() {
	return _swapChain->GetCurrentBackBufferIndex();
}

void SwapChainD3D12::present(u32 syncInterval, u32 flags) {
	_swapChain->Present(syncInterval, flags);
}

void SwapChainD3D12::getBackBuffer(Resource** resource, u32 index) {
	ID3D12Resource** dest = &static_cast<ResourceD3D12*>(*resource)->_resource;
	_swapChain->GetBuffer(index, IID_PPV_ARGS(dest));
}


void CommandQueueD3D12::initialize(const CommandQueueDesc& desc) {
	ID3D12Device2* device = toD3d12(desc._device);
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = static_cast<D3D12_COMMAND_LIST_TYPE>(desc._type);
	LTN_SUCCEEDED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

	//フェンス生成
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (_fenceEvent == nullptr) {
		HRESULT_FROM_WIN32(GetLastError());
	}

	SetDebugName(_commandQueue, desc._debugName);
	SetDebugName(_fence, desc._debugName);
}

void CommandQueueD3D12::terminate() {
	CloseHandle(_fenceEvent);
	_fence->Release();
	_commandQueue->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

void CommandQueueD3D12::executeCommandLists(u32 count, CommandList** commandLists) {
	constexpr u32 COMMAND_LIST_COUNT_MAX = 16;
	ID3D12CommandList* lists[COMMAND_LIST_COUNT_MAX] = {};
	LTN_ASSERT(count <= COMMAND_LIST_COUNT_MAX);
	for (u32 commandListIndex = 0; commandListIndex < count; ++commandListIndex) {
		ID3D12GraphicsCommandList* commandList = static_cast<CommandListD3D12*>(commandLists[commandListIndex])->_commandList;
		commandList->Close();
		lists[commandListIndex] = commandList;
	}

	_commandQueue->ExecuteCommandLists(count, lists);

	// フェンス値を更新
	u64 fenceValue = incrimentFence();
	for (u32 commandListIndex = 0; commandListIndex < count; ++commandListIndex) {
		*static_cast<CommandListD3D12*>(commandLists[commandListIndex])->_fenceValue = fenceValue;
	}
}

void CommandQueueD3D12::getTimestampFrequency(u64* frequency) {
	_commandQueue->GetTimestampFrequency(frequency);
}

bool CommandQueueD3D12::isFenceComplete(u64 fenceValue) {
	if (fenceValue > _lastFenceValue) {
		_lastFenceValue = Max(_lastFenceValue, _fence->GetCompletedValue());
	}

	return fenceValue <= _lastFenceValue;
}

void CommandQueueD3D12::waitForIdle() {
	u64 fenceValue = incrimentFence();
	waitForFence(fenceValue);
}

void CommandQueueD3D12::waitForFence(u64 fenceValue) {
	//引数のフェンス値が最新のフェンス値以下であればすでに終了している
	if (isFenceComplete(fenceValue)) {
		return;
	}

	//フェンスが完了するまで待機
	_fence->SetEventOnCompletion(fenceValue, _fenceEvent);
	WaitForSingleObjectEx(_fenceEvent, INFINITE, FALSE);
	_lastFenceValue = fenceValue;
}

u64 CommandQueueD3D12::incrimentFence() {
	_commandQueue->Signal(_fence, _nextFenceValue);
	return _nextFenceValue++;
}

u64 CommandQueueD3D12::getCompleatedValue() const {
	return _fence->GetCompletedValue();
}

HardwareFactory* GraphicsApiInstanceAllocatorImpl::allocateHardwareFactroy() {
	return _hardwareFactories.requestData();
}

HardwareAdapter* GraphicsApiInstanceAllocatorImpl::allocateHardwareAdapter() {
	return _hardwareAdapters.requestData();
}

Device* GraphicsApiInstanceAllocatorImpl::allocateDevice() {
	return _devices.requestData();
}

SwapChain* GraphicsApiInstanceAllocatorImpl::allocateSwapChain() {
	return _swapChains.requestData();
}

CommandQueue* GraphicsApiInstanceAllocatorImpl::allocateCommandQueue() {
	return _commandQueues.requestData();
}

CommandList* GraphicsApiInstanceAllocatorImpl::allocateCommandList(u64 fenceValue) {
	u32 commandListCount = _commandLists.getArrayCountMax();
	for (u32 commandListIndex = 0; commandListIndex < commandListCount; ++commandListIndex) {
		if (_commandLists._flags[commandListIndex] == GRAPHICS_INTERFACE_STATE_NONE) {
			continue;
		}

		if (_commandListFenceValues[commandListIndex] < fenceValue) {
			return &_commandLists[commandListIndex];
		}
	}

	u32 index = _commandLists.request();
	u64* fenceValuePtr = &_commandListFenceValues[index];
	*fenceValuePtr = CommandListD3D12::INVAILD_FENCE_VALUE;
	_commandLists.resetStateFlags(index);

	CommandListD3D12* commandList = &_commandLists[index];
	commandList->_fenceValue = fenceValuePtr;
	return commandList;
}

Resource* GraphicsApiInstanceAllocatorImpl::allocateResource() {
	return _resources.requestData();
}

DescriptorHeap* GraphicsApiInstanceAllocatorImpl::allocateDescriptorHeap() {
	return _descriptorHeaps.requestData();
}

ShaderBlob* GraphicsApiInstanceAllocatorImpl::allocateShaderBlob() {
	return _shaderBlobs.requestData();
}

PipelineState* GraphicsApiInstanceAllocatorImpl::allocatePipelineState() {
	return _pipelineStates.requestData();
}

RootSignature* GraphicsApiInstanceAllocatorImpl::allocateRootSignature() {
	return _rootSignatures.requestData();
}

CommandSignature* GraphicsApiInstanceAllocatorImpl::allocateCommandSignature() {
	return _commandSignatures.requestData();
}

QueryHeap* GraphicsApiInstanceAllocatorImpl::allocateQueryHeap() {
	return _queryHeaps.requestData();
}

void GraphicsApiInstanceAllocatorImpl::initialize() {
	_hardwareFactories.initialize();
	_hardwareAdapters.initialize();
	_devices.initialize();
	_swapChains.initialize();
	_commandQueues.initialize();
	_commandLists.initialize();
	_resources.initialize();
	_descriptorHeaps.initialize();
	_shaderBlobs.initialize();
	_pipelineStates.initialize();
	_rootSignatures.initialize();
	_commandSignatures.initialize();
	_queryHeaps.initialize();
}

void GraphicsApiInstanceAllocatorImpl::terminate() {
	u32 commandListCount = _commandLists.getArrayCountMax();
	for (u32 commandListIndex = 0; commandListIndex < commandListCount; ++commandListIndex) {
		if (_commandLists._flags[commandListIndex] == GRAPHICS_INTERFACE_STATE_ENABLE) {
			_commandLists[commandListIndex].terminate();
			_commandLists.discard(commandListIndex);
		}
	}

	LTN_ASSERT(_hardwareFactories.getInstanceCount() == 0);
	LTN_ASSERT(_hardwareAdapters.getInstanceCount() == 0);
	LTN_ASSERT(_devices.getInstanceCount() == 0);
	LTN_ASSERT(_swapChains.getInstanceCount() == 0);
	LTN_ASSERT(_commandQueues.getInstanceCount() == 0);
	LTN_ASSERT(_commandLists.getInstanceCount() == 0);
	LTN_ASSERT(_resources.getInstanceCount() == 0);
	LTN_ASSERT(_descriptorHeaps.getInstanceCount() == 0);
	LTN_ASSERT(_shaderBlobs.getInstanceCount() == 0);
	LTN_ASSERT(_pipelineStates.getInstanceCount() == 0);
	LTN_ASSERT(_rootSignatures.getInstanceCount() == 0);
	LTN_ASSERT(_commandSignatures.getInstanceCount() == 0);
	LTN_ASSERT(_queryHeaps.getInstanceCount() == 0);

	_hardwareFactories.terminate();
	_hardwareAdapters.terminate();
	_devices.terminate();
	_swapChains.terminate();
	_commandQueues.terminate();
	_commandLists.terminate();
	_resources.terminate();
	_descriptorHeaps.terminate();
	_shaderBlobs.terminate();
	_pipelineStates.terminate();
	_rootSignatures.terminate();
	_commandSignatures.terminate();
	_queryHeaps.terminate();

	// リークしているGPUリソースをチェックして表示
	IDXGIDebug1* debug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
}

void GraphicsApiInstanceAllocatorImpl::update() {
	// リソース
	u32 resourceCount = _resources.getArrayCountMax();
	for (u32 resourceIndex = 0; resourceIndex < resourceCount; ++resourceIndex) {
		if (_resources._flags[resourceIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_resources.discard(resourceIndex);
		}
	}

	// シェーダーブロブ
	u32 shaderBlobCount = _shaderBlobs.getArrayCountMax();
	for (u32 shaderBlobIndex = 0; shaderBlobIndex < shaderBlobCount; ++shaderBlobIndex) {
		if (_shaderBlobs._flags[shaderBlobIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_shaderBlobs.discard(shaderBlobIndex);
		}
	}

	// デスクリプターヒープ
	u32 descriptorHeapCount = _descriptorHeaps.getArrayCountMax();
	for (u32 descriptorHeapIndex = 0; descriptorHeapIndex < descriptorHeapCount; ++descriptorHeapIndex) {
		if (_descriptorHeaps._flags[descriptorHeapIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_descriptorHeaps.discard(descriptorHeapIndex);
		}
	}

	// コマンドキュー
	u32 commandQueueCount = _commandQueues.getArrayCountMax();
	for (u32 commandQueueIndex = 0; commandQueueIndex < commandQueueCount; ++commandQueueIndex) {
		if (_commandQueues._flags[commandQueueIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_commandQueues.discard(commandQueueIndex);
		}
	}

	// スワップチェイン
	u32 swapChainCount = _swapChains.getArrayCountMax();
	for (u32 swapChainIndex = 0; swapChainIndex < swapChainCount; ++swapChainIndex) {
		if (_swapChains._flags[swapChainIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_swapChains.discard(swapChainIndex);
		}
	}

	// デバイス
	u32 deviceCount = _devices.getArrayCountMax();
	for (u32 deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
		if (_devices._flags[deviceIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_devices.discard(deviceIndex);
		}
	}

	// ハードウェアアダプタ
	u32 adapterCount = _hardwareAdapters.getArrayCountMax();
	for (u32 adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
		if (_hardwareAdapters._flags[adapterIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_hardwareAdapters.discard(adapterIndex);
		}
	}

	// ハードウェアファクトリー
	u32 factoryCount = _hardwareFactories.getArrayCountMax();
	for (u32 factoryIndex = 0; factoryIndex < factoryCount; ++factoryIndex) {
		if (_hardwareFactories._flags[factoryIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_hardwareFactories.discard(factoryIndex);
		}
	}

	// パイプラインステート
	u32 pipelineStateCount = _pipelineStates.getArrayCountMax();
	for (u32 pipelineStateIndex = 0; pipelineStateIndex < pipelineStateCount; ++pipelineStateIndex) {
		if (_pipelineStates._flags[pipelineStateIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_pipelineStates.discard(pipelineStateIndex);
		}
	}

	// ルートシグネチャー
	u32 rootSignatureCount = _rootSignatures.getArrayCountMax();
	for (u32 rootSignatureIndex = 0; rootSignatureIndex < rootSignatureCount; ++rootSignatureIndex) {
		if (_rootSignatures._flags[rootSignatureIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_rootSignatures.discard(rootSignatureIndex);
		}
	}

	// コマンドシグネチャ
	u32 commandSignatureCount = _commandSignatures.getArrayCountMax();
	for (u32 commandSignatureIndex = 0; commandSignatureIndex < commandSignatureCount; ++commandSignatureIndex) {
		if (_commandSignatures._flags[commandSignatureIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_commandSignatures.discard(commandSignatureIndex);
		}
	}

	// クエリヒープ
	u32 queryHeapCount = _queryHeaps.getArrayCountMax();
	for (u32 queryHeapIndex = 0; queryHeapIndex < queryHeapCount; ++queryHeapIndex) {
		if (_queryHeaps._flags[queryHeapIndex] == GRAPHICS_INTERFACE_STATE_REQUEST_DELETE) {
			_queryHeaps.discard(queryHeapIndex);
		}
	}
}

GraphicsApiInstanceAllocatorImpl* GraphicsApiInstanceAllocatorImpl::Get() {
	return &_graphicsApiInstanceAllocator;
}

GraphicsApiInstanceAllocator* GraphicsApiInstanceAllocator::Get() {
	return &_graphicsApiInstanceAllocator;
}

void DescriptorHeapD3D12::initialize(const DescriptorHeapDesc& desc) {
	ID3D12Device* device = static_cast<DeviceD3D12*>(desc._device)->_device;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = desc._numDescriptors;
	heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(desc._type);
	heapDesc.Flags = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(desc._flags);
	LTN_SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_descriptorHeap)));
}

void DescriptorHeapD3D12::terminate() {
	_descriptorHeap->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

u32 DescriptorHeapD3D12::getIncrimentSize() const {
	return u32();
}

CpuDescriptorHandle DescriptorHeapD3D12::getCPUDescriptorHandleForHeapStart() const {
	CpuDescriptorHandle handle;
	handle._ptr = _descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	return handle;
}

GpuDescriptorHandle DescriptorHeapD3D12::getGPUDescriptorHandleForHeapStart() const {
	GpuDescriptorHandle handle;
	handle._ptr = _descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
	return handle;
}

void CommandListD3D12::initialize(const CommandListDesc& desc) {
	ID3D12Device2* device = toD3d12(desc._device);
	D3D12_COMMAND_LIST_TYPE type = static_cast<D3D12_COMMAND_LIST_TYPE>(desc._type);

	if (*_fenceValue == INVAILD_FENCE_VALUE) {
		LTN_ASSERT(_allocator == nullptr);
		LTN_ASSERT(_commandList == nullptr);
		LTN_SUCCEEDED(device->CreateCommandAllocator(type, IID_PPV_ARGS(&_allocator)));
		LTN_SUCCEEDED(device->CreateCommandList(0, type, _allocator, nullptr, IID_PPV_ARGS(&_commandList)));
		_commandList->Close();

		SetDebugName(_commandList, desc._debugName);
		SetDebugName(_allocator, desc._debugName);
	}

	LTN_ASSERT(_allocator != nullptr);
	LTN_ASSERT(_commandList != nullptr);

	_allocator->Reset();
	_commandList->Reset(_allocator, nullptr);
}

void CommandListD3D12::terminate() {
	_allocator->Release();
	_commandList->Release();
}

void CommandListD3D12::transitionBarrierSimple(Resource* resource, ResourceStates currentState, ResourceStates nextState) {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = toD3d12(resource);
	barrier.Transition.StateBefore = toD3d12(currentState);
	barrier.Transition.StateAfter = toD3d12(nextState);
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	_commandList->ResourceBarrier(1, &barrier);
}

void CommandListD3D12::transitionBarriers(ResourceTransitionBarrier* barriers, u32 count) {
	constexpr u32 BARRIER_COUNT_MAX = 128;
	LTN_ASSERT(count > 0);
	LTN_ASSERT(count < BARRIER_COUNT_MAX);
	D3D12_RESOURCE_BARRIER barriersD3d12[BARRIER_COUNT_MAX] = {};
	for (u32 barrierIndex = 0; barrierIndex < count; ++barrierIndex) {
		ResourceTransitionBarrier& sourceBarrier = barriers[barrierIndex];
		D3D12_RESOURCE_BARRIER& barrier = barriersD3d12[barrierIndex];
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = toD3d12(sourceBarrier._resource);
		barrier.Transition.StateBefore = toD3d12(sourceBarrier._stateBefore);
		barrier.Transition.StateAfter = toD3d12(sourceBarrier._stateAfter);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}
	_commandList->ResourceBarrier(count, barriersD3d12);
}

void CommandListD3D12::copyBufferRegion(Resource* dstBuffer, u64 dstOffset, Resource* srcBuffer, u64 srcOffset, u64 numBytes) {
	_commandList->CopyBufferRegion(toD3d12(dstBuffer), dstOffset, toD3d12(srcBuffer), srcOffset, numBytes);
}

void CommandListD3D12::copyTextureRegion(const TextureCopyLocation* dst, u32 dstX, u32 dstY, u32 dstZ, const TextureCopyLocation* src, const Box* srcBox) {
	D3D12_TEXTURE_COPY_LOCATION dstD3d12 = toD3d12(*dst);
	D3D12_TEXTURE_COPY_LOCATION srcD3d12 = toD3d12(*src);
	_commandList->CopyTextureRegion(&dstD3d12, dstX, dstY, dstZ, &srcD3d12, toD3d12(srcBox));
}

void CommandListD3D12::copyResource(Resource* dstResource, Resource* srcResource) {
	_commandList->CopyResource(toD3d12(dstResource), toD3d12(srcResource));
}

void CommandListD3D12::setDescriptorHeaps(u32 count, DescriptorHeap** descriptorHeaps) {
	constexpr u32 DESCRIPTOR_HEAP_COUNT_MAX = 3;
	ID3D12DescriptorHeap* heaps[DESCRIPTOR_HEAP_COUNT_MAX] = {};
	LTN_ASSERT(count <= DESCRIPTOR_HEAP_COUNT_MAX);
	for (u32 descriptorHeapIndex = 0; descriptorHeapIndex < count; ++descriptorHeapIndex) {
		heaps[descriptorHeapIndex] = static_cast<DescriptorHeapD3D12*>(descriptorHeaps[descriptorHeapIndex])->_descriptorHeap;
	}

	_commandList->SetDescriptorHeaps(count, heaps);
}

void CommandListD3D12::setViewports(u32 count, const ViewPort* viewPorts) {
	constexpr u32 VIEWPORT_COUNT_MAX = 8;
	D3D12_VIEWPORT views[VIEWPORT_COUNT_MAX] = {};
	LTN_ASSERT(count <= VIEWPORT_COUNT_MAX);
	for (u32 viewIndex = 0; viewIndex < count; ++viewIndex) {
		const ViewPort& src = viewPorts[viewIndex];
		D3D12_VIEWPORT& view = views[viewIndex];
		view.Width = src._width;
		view.Height = src._height;
		view.TopLeftX = src._topLeftX;
		view.TopLeftY = src._topLeftY;
		view.MinDepth = src._minDepth;
		view.MaxDepth = src._maxDepth;
	}

	_commandList->RSSetViewports(count, views);
}

void CommandListD3D12::setScissorRects(u32 count, const Rect* scissorRects) {
	constexpr u32 RECT_COUNT_MAX = 8;
	D3D12_RECT rects[RECT_COUNT_MAX] = {};
	LTN_ASSERT(count <= RECT_COUNT_MAX);
	for (u32 rectIndex = 0; rectIndex < count; ++rectIndex) {
		const Rect& src = scissorRects[rectIndex];
		D3D12_RECT& rect = rects[rectIndex];
		rect.top = src._top;
		rect.bottom = src._bottom;
		rect.right = src._right;
		rect.left = src._left;
	}

	_commandList->RSSetScissorRects(count, rects);
}

void CommandListD3D12::setRenderTargets(u32 count, DescriptorHandle* rtvHandles, DescriptorHandle* dsvHandle) {
	constexpr u32 RTV_COUNT_MAX = 16;
	D3D12_CPU_DESCRIPTOR_HANDLE handles[RTV_COUNT_MAX] = {};
	LTN_ASSERT(count <= RTV_COUNT_MAX);
	for (u32 rtvIndex = 0; rtvIndex < count; ++rtvIndex) {
		handles[rtvIndex].ptr = rtvHandles[rtvIndex]._cpuHandle._ptr;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE dsv;
	D3D12_CPU_DESCRIPTOR_HANDLE* dsvPtr = nullptr;
	if (dsvHandle != nullptr) {
		dsv.ptr = dsvHandle->_cpuHandle._ptr;
		dsvPtr = &dsv;
	}
	_commandList->OMSetRenderTargets(count, handles, FALSE, dsvPtr);
}

void CommandListD3D12::clearRenderTargetView(DescriptorHandle rtvHandle, f32 clearColor[4]) {
	_commandList->ClearRenderTargetView(toD3d12(rtvHandle._cpuHandle), clearColor, 0, nullptr);
}

void CommandListD3D12::clearDepthStencilView(CpuDescriptorHandle depthStencilView, ClearFlags clearFlags, f32 depth, u8 stencil, u32 numRects, const Rect* rects) {
	_commandList->ClearDepthStencilView(toD3d12(depthStencilView), toD3d12(clearFlags), depth, stencil, numRects, toD3d12(rects));
}

void CommandListD3D12::setPipelineState(PipelineState* pipelineState) {
	ID3D12PipelineState* pipelineStateD3d12 = static_cast<PipelineStateD3D12*>(pipelineState)->_pipelineState;
	_commandList->SetPipelineState(pipelineStateD3d12);
}

void CommandListD3D12::setGraphicsRootSignature(RootSignature* rootSignature) {
	_commandList->SetGraphicsRootSignature(toD3d12(rootSignature));
}

void CommandListD3D12::setComputeRootSignature(RootSignature* rootSignature) {
	_commandList->SetComputeRootSignature(toD3d12(rootSignature));
}

void CommandListD3D12::setPrimitiveTopology(PrimitiveTopology primitiveTopology) {
	D3D12_PRIMITIVE_TOPOLOGY topology = static_cast<D3D12_PRIMITIVE_TOPOLOGY>(primitiveTopology);
	_commandList->IASetPrimitiveTopology(topology);
}

void CommandListD3D12::setGraphicsRootDescriptorTable(u32 rootParameterIndex, GpuDescriptorHandle baseDescriptor) {
	_commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, toD3d12(baseDescriptor));
}

void CommandListD3D12::setComputeRootDescriptorTable(u32 rootParameterIndex, GpuDescriptorHandle baseDescriptor) {
	_commandList->SetComputeRootDescriptorTable(rootParameterIndex, toD3d12(baseDescriptor));
}

void CommandListD3D12::setGraphicsRoot32BitConstants(u32 rootParameterIndex, u32 num32BitValuesToSet, const void* srcData, u32 destOffsetIn32BitValues) {
	_commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void CommandListD3D12::drawInstanced(u32 vertexCountPerInstance, u32 instanceCount, u32 startVertexLocation, u32 startInstanceLocation) {
	_commandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void CommandListD3D12::drawIndexedInstanced(u32 indexCountPerInstance, u32 instanceCount, u32 startIndexLocation, s32 baseVertexLocation, u32 startInstanceLocation) {
	_commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

#if ENABLE_MESH_SHADER
void CommandListD3D12::dispatchMesh(u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) {
	_commandList->DispatchMesh(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}
#endif

void CommandListD3D12::dispatch(u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) {
	_commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void CommandListD3D12::executeIndirect(CommandSignature* commandSignature, u32 maxCommandCount, Resource* argumentBuffer, u64 argumentBufferOffset, Resource* countBuffer, u64 countBufferOffset) {
	_commandList->ExecuteIndirect(toD3d12(commandSignature), maxCommandCount, toD3d12(argumentBuffer), argumentBufferOffset, toD3d12(countBuffer), countBufferOffset);
}

void CommandListD3D12::clearUnorderedAccessViewUint(GpuDescriptorHandle viewGPUHandleInCurrentHeap, CpuDescriptorHandle viewCPUHandle, Resource* resource, const u32 values[4], u32 numRects, const Rect* rects) {
	_commandList->ClearUnorderedAccessViewUint(toD3d12(viewGPUHandleInCurrentHeap), toD3d12(viewCPUHandle), toD3d12(resource), values, numRects, toD3d12(rects));
}

void CommandListD3D12::setVertexBuffers(u32 startSlot, u32 numViews, const VertexBufferView* views) {
	_commandList->IASetVertexBuffers(startSlot, numViews, toD3d12(views));
}

void CommandListD3D12::setIndexBuffer(const IndexBufferView* view) {
	_commandList->IASetIndexBuffer(toD3d12(view));
}

void CommandListD3D12::endQuery(QueryHeap* queryHeap, QueryType type, u32 index) {
	_commandList->EndQuery(toD3d12(queryHeap), static_cast<D3D12_QUERY_TYPE>(type), index);
}

void CommandListD3D12::resolveQueryData(QueryHeap* queryHeap, QueryType type, u32 startIndex, u32 numQueries, Resource* destinationBuffer, u64 alignedDestinationBufferOffset) {
	_commandList->ResolveQueryData(toD3d12(queryHeap), static_cast<D3D12_QUERY_TYPE>(type), startIndex, numQueries, toD3d12(destinationBuffer), alignedDestinationBufferOffset);
}

void CommandSignatureD3D12::initialize(const CommandSignatureDesc& desc) {
	ID3D12Device2* device = toD3d12(desc._device);
	ID3D12RootSignature* rootSignature = nullptr;
	if (desc._rootSignature != nullptr) {
		rootSignature = toD3d12(desc._rootSignature);
	}
	D3D12_COMMAND_SIGNATURE_DESC descD3d12 = toD3d12(desc);
	device->CreateCommandSignature(&descD3d12, rootSignature, IID_PPV_ARGS(&_commandSignature));
}

void CommandSignatureD3D12::terminate() {
	_commandSignature->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

void ResourceD3D12::terminate() {
	_resource->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

void ResourceD3D12::unmap(const MemoryRange* range) {
	const D3D12_RANGE* memoryRange = reinterpret_cast<const D3D12_RANGE*>(range);
	_resource->Unmap(0, memoryRange);
}

u64 ResourceD3D12::getGpuVirtualAddress() const {
	return _resource->GetGPUVirtualAddress();
}

void ResourceD3D12::setDebugName(const char* name) {
	SetDebugName(_resource, name);
}

void* ResourceD3D12::map(const MemoryRange* range) {
	const D3D12_RANGE* memoryRange = reinterpret_cast<const D3D12_RANGE*>(range);
	void* ptr = nullptr;
	_resource->Map(0, memoryRange, &ptr);
	return ptr;
}

bool IsSupportedMeshShader(Device* device) {
#if ENABLE_MESH_SHADER
	ID3D12Device2* deviceD3d12 = toD3d12(device);
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
	if (FAILED(deviceD3d12->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
		|| (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5)) {
		LTN_INFO("ERROR: Shader Model 6.5 is not supported\n");
		return false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
	if (FAILED(deviceD3d12->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features)))
		|| (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)) {
		LTN_INFO("ERROR: Mesh Shaders aren't supported!\n");
		return false;
	}

#endif
	return true;

}

u32 GetConstantBufferAligned(u32 sizeInByte) {
	return GetAligned(sizeInByte, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}

u32 GetTextureBufferAligned(u32 sizeInByte) {
	return GetAligned(sizeInByte, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
}

void PipelineStateD3D12::iniaitlize(const GraphicsPipelineStateDesc& desc) {
	ID3D12Device2* device = toD3d12(desc._device);

	auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthFunc = toD3d12(desc._depthComparisonFunc);
	depthStencilDesc.DepthWriteMask = toD3d12(desc._depthWriteMask);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.NumElements = desc._inputElementCount;
	psoDesc.InputLayout.pInputElementDescs = toD3d12(desc._inputElements);
	psoDesc.pRootSignature = toD3d12(desc._rootSignature);
	psoDesc.VS = toD3d12(desc._vs);
	psoDesc.PS = toD3d12(desc._ps);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(desc._topologyType);
	psoDesc.DSVFormat = toD3d12(desc._dsvFormat);
	psoDesc.SampleDesc = DefaultSampleDesc();

	LTN_ASSERT(desc._numRenderTarget <= 8);
	psoDesc.NumRenderTargets = desc._numRenderTarget;
	for (u32 renderTargetIndex = 0; renderTargetIndex < desc._numRenderTarget; ++renderTargetIndex) {
		psoDesc.RTVFormats[renderTargetIndex] = toD3d12(desc._rtvFormats[renderTargetIndex]);
	}

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState));
}

#if ENABLE_MESH_SHADER
void PipelineStateD3D12::iniaitlize(const MeshPipelineStateDesc& desc) {
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = desc._numRenderTarget;
	for (u32 renderTargetIndex = 0; renderTargetIndex < desc._numRenderTarget; ++renderTargetIndex) {
		rtvFormats.RTFormats[renderTargetIndex] = toD3d12(desc._rtvFormats[renderTargetIndex]);
	}

	auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthFunc = toD3d12(desc._depthComparisonFunc);

	auto rasterrizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterrizerState.FillMode = toD3d12(desc._fillMode);

	MeshShaderPsoDesc psoDesc = {};
	psoDesc.pRootSignature = toD3d12(desc._rootSignature);
	psoDesc.MS = toD3d12(desc._ms);
	psoDesc.AS = toD3d12(desc._as);
	psoDesc.PS = toD3d12(desc._ps);
	psoDesc.RTFormats = rtvFormats;
	psoDesc.DepthFormat = toD3d12(desc._dsvFormat);
	psoDesc.RasterizerState = rasterrizerState;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(toD3d12(desc._blendDesc));
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.SampleDesc = DefaultSampleDesc();

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoDesc;
	streamDesc.SizeInBytes = sizeof(psoDesc);

	ID3D12Device2* device = toD3d12(desc._device);
	LTN_SUCCEEDED(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&_pipelineState)));
}
#endif

void PipelineStateD3D12::iniaitlize(const ComputePipelineStateDesc& desc) {
	ID3D12Device2* device = toD3d12(desc._device);
	D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc = {};
	pipelineStateDesc.pRootSignature = toD3d12(desc._rootSignature);
	pipelineStateDesc.CS = toD3d12(desc._cs);
	pipelineStateDesc.NodeMask = desc._nodeMask;
	pipelineStateDesc.CachedPSO.CachedBlobSizeInBytes = desc._cachedPSO._cachedBlobSizeInBytes;
	pipelineStateDesc.CachedPSO.pCachedBlob = desc._cachedPSO.cachedBlob;
	pipelineStateDesc.Flags = toD3d12(desc._flags);
	device->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(&_pipelineState));
}

void PipelineStateD3D12::terminate() {
	_pipelineState->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

void PipelineStateD3D12::setDebugName(const char* name) {
	SetDebugName(_pipelineState, name);
}

void RootSignatureD3D12::iniaitlize(const RootSignatureDesc& desc) {
	ID3D12Device2* device = toD3d12(desc._device);

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	rootSignatureDesc.Desc_1_1.Flags = static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(desc._flags);

	constexpr u32 ROOT_PARAMETER_COUNT_MAX = 32;
	D3D12_ROOT_PARAMETER1 rootParameters[ROOT_PARAMETER_COUNT_MAX] = {};
	for (u32 parameterIndex = 0; parameterIndex < desc._numParameters; ++parameterIndex) {
		const RootParameter& parameter = desc._parameters[parameterIndex];
		D3D12_ROOT_PARAMETER1& parameterD3d12 = rootParameters[parameterIndex];
		parameterD3d12.ParameterType = static_cast<D3D12_ROOT_PARAMETER_TYPE>(parameter._parameterType);
		parameterD3d12.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(parameter._shaderVisibility);
		switch (parameter._parameterType) {
		case ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			parameterD3d12.DescriptorTable.NumDescriptorRanges = parameter._descriptorTable._numDescriptorRanges;
			parameterD3d12.DescriptorTable.pDescriptorRanges = reinterpret_cast<const D3D12_DESCRIPTOR_RANGE1*>(parameter._descriptorTable._descriptorRanges);
			break;
		case ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			parameterD3d12.Constants.Num32BitValues = parameter._constants._num32BitValues;
			parameterD3d12.Constants.RegisterSpace = parameter._constants._registerSpace;
			parameterD3d12.Constants.ShaderRegister = parameter._constants._shaderRegister;
			break;
		case ROOT_PARAMETER_TYPE_SRV:
		case ROOT_PARAMETER_TYPE_CBV:
		case ROOT_PARAMETER_TYPE_UAV:
			parameterD3d12.Descriptor.ShaderRegister = parameter._descriptor._shaderRegister;
			parameterD3d12.Descriptor.RegisterSpace = parameter._descriptor._registerSpace;
			parameterD3d12.Descriptor.Flags = static_cast<D3D12_ROOT_DESCRIPTOR_FLAGS>(parameter._descriptor._flags);
			break;
		}
	}

	D3D12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
	{
		D3D12_STATIC_SAMPLER_DESC& samplerDesc = samplerDescs[0];
		samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; 
	}
	{
		D3D12_STATIC_SAMPLER_DESC& samplerDesc = samplerDescs[1];
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = 0.0f;
		samplerDesc.ShaderRegister = 1;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	rootSignatureDesc.Desc_1_1.NumParameters = desc._numParameters;
	rootSignatureDesc.Desc_1_1.pParameters = rootParameters;
	rootSignatureDesc.Desc_1_1.pStaticSamplers = samplerDescs;
	rootSignatureDesc.Desc_1_1.NumStaticSamplers = LTN_COUNTOF(samplerDescs);

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	if (error != nullptr) {
		char errorMessage[512];
		memcpy(errorMessage, error->GetBufferPointer(), LTN_COUNTOF(errorMessage));
		printf(errorMessage);
		error->Release();
	}
	LTN_ASSERT(error == nullptr);

	device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));

	signature->Release();
}

void RootSignatureD3D12::terminate() {
	_rootSignature->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

void RootSignatureD3D12::setDebugName(const char* name) {
	SetDebugName(_rootSignature, name);
	_debugName = name;
}

void ShaderBlobD3D12::initialize(const char* filePath) {
	constexpr u32 SET_NAME_LENGTH_COUNT_MAX = 256;
	WCHAR wName[SET_NAME_LENGTH_COUNT_MAX] = {};
	size_t wLength = 0;
	mbstowcs_s(&wLength, wName, SET_NAME_LENGTH_COUNT_MAX, filePath, _TRUNCATE);
	D3DReadFileToBlob(wName, &_blob);
}

void ShaderBlobD3D12::terminate() {
	_blob->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}

ShaderByteCode ShaderBlobD3D12::getShaderByteCode() const {
	ShaderByteCode byteCode = {};
	byteCode._bytecodeLength = _blob->GetBufferSize();
	byteCode._shaderBytecode = _blob->GetBufferPointer();
	return byteCode;
}

void QueryHeapD3D12::initialize(const QueryHeapDesc& desc) {
	ID3D12Device* device = toD3d12(desc._device);
	D3D12_QUERY_HEAP_DESC descD3d12 = {};
	descD3d12.Count = desc._count;
	descD3d12.NodeMask = desc._nodeMask;
	descD3d12.Type = static_cast<D3D12_QUERY_HEAP_TYPE>(desc._type);
	LTN_SUCCEEDED(device->CreateQueryHeap(&descD3d12, IID_PPV_ARGS(&_queryHeap)));
}

void QueryHeapD3D12::terminate() {
	_queryHeap->Release();
	*_stateFlags = GRAPHICS_INTERFACE_STATE_REQUEST_DELETE;
}


namespace DebugMarker {
	void setMarker(CommandList* commandList, const Color4& color, const char* name, va_list va) {
		char nameBuffer[SET_NAME_LENGTH_COUNT_MAX] = {};
		vsprintf_s(nameBuffer, name, va);

		u8 r = static_cast<u8>(color._r * 255);
		u8 g = static_cast<u8>(color._g * 255);
		u8 b = static_cast<u8>(color._b * 255);
		u64 pixColor = PIX_COLOR(r, g, b);
		PIXSetMarker(toD3d12(commandList), pixColor, nameBuffer);
	}

	void pushMarker(CommandList* commandList, const Color4& color, const char* name, va_list va) {
		char nameBuffer[SET_NAME_LENGTH_COUNT_MAX] = {};
		vsprintf_s(nameBuffer, name, va);

		u8 r = static_cast<u8>(color._r * 255);
		u8 g = static_cast<u8>(color._g * 255);
		u8 b = static_cast<u8>(color._b * 255);
		u64 pixColor = PIX_COLOR(r, g, b);
		PIXBeginEvent(toD3d12(commandList), pixColor, nameBuffer);
	}

	ScopedEvent::ScopedEvent(CommandList* commandList, const Color4& color, const char* name, ...) {
		va_list va;
		va_start(va, name);
		setEvent(commandList, color, name, va);
		va_end(va);
	}

	ScopedEvent::~ScopedEvent() {
		popMarker(_commandList);
	}

	void ScopedEvent::setEvent(CommandList* commandList, const Color4& color, const char* name, va_list va) {
		_commandList = commandList;
		pushMarker(commandList, color, name, va);
	}

	void setMarker(CommandList* commandList, const Color4& color, const char* name, ...) {
		va_list va;
		va_start(va, name);
		setMarker(commandList, color, name, va);
		va_end(va);
	}
	void pushMarker(CommandList* commandList, const Color4& color, const char* name, ...) {
		char nameBuffer[SET_NAME_LENGTH_COUNT_MAX] = {};
		va_list va;
		va_start(va, name);
		vsprintf_s(nameBuffer, name, va);
		va_end(va);

		u8 r = static_cast<u8>(color._r * 255);
		u8 g = static_cast<u8>(color._g * 255);
		u8 b = static_cast<u8>(color._b * 255);
		u64 pixColor = PIX_COLOR(r, g, b);
		PIXBeginEvent(toD3d12(commandList), pixColor, nameBuffer);
	}
	void popMarker(CommandList* commandList) {
		PIXEndEvent(toD3d12(commandList));
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace DebugGui {
	LTN_STATIC_ASSERT(TEXTURE_SAMPLE_TYPE_COUNT == ImTextureSamplerType_Count, "TEXTURE_SAMPLER_TYPE_COUNT different.");
	LTN_STATIC_ASSERT(COLOR_CHANNEL_FILTER_COUNT == ImColorChannelFilter_Count, "COLOR_CHANNEL_FILTER_COUNT different.");

	void InitializeDebugWindowGui(const DebugWindowDesc& desc) {
		ID3D12Device2* device = toD3d12(desc._device);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer bindings
		ImGui_ImplWin32_Init(reinterpret_cast<HWND>(desc._hWnd));
		ImGui_ImplDX12_Init(device,
			desc._bufferingCount,
			static_cast<DXGI_FORMAT>(desc._rtvFormat),
			toD3d12(desc._descriptorHeap),
			toD3d12(desc._srvHandle._cpuHandle),
			toD3d12(desc._srvHandle._gpuHandle));
	}

	void BeginDebugWindowGui() {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void RenderDebugWindowImgui() {
		ImGui::Render();
	}

	void RenderDebugWindowGui(CommandList* commandList) {
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), toD3d12(commandList));
	}

	bool TranslateWindowProc(s32* hWnd, u32 message, u64 wParam, s64 lParam) {
		return ImGui_ImplWin32_WndProcHandler(reinterpret_cast<HWND>(hWnd), message, wParam, lParam);
	}

	void Start(const char* windowName) {
		ImGui::Begin(windowName);
	}

	void End() {
		ImGui::End();
	}

	void TerminateDebugWindowGui() {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Text(const char* text, ...) {
		va_list args;
		va_start(args, text);
		ImGui::TextV(text, args);
		va_end(args);
	}

	void TextColored(const Color4& col, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		ImGui::TextColoredV(*reinterpret_cast<const ImVec4*>(&col), fmt, args);
		va_end(args);
	}

	void TextDisabled(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		ImGui::TextDisabledV(fmt, args);
		va_end(args);
	}

	bool DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, float power) {
		return ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, power);
	}

	bool DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, float power) {
		return ImGui::DragFloat2(label, v, v_speed, v_min, v_max, format, power);
	}

	bool DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, float power) {
		return ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, power);
	}

	bool DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, float power) {
		return ImGui::DragFloat4(label, v, v_speed, v_min, v_max, format, power);
	}

	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format) {
		return ImGui::SliderInt(label, v, v_min, v_max, format);
	}

	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float power) {
		return ImGui::SliderFloat(label, v, v_min, v_max, format, power);
	}

	bool SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, float power) {
		return ImGui::SliderFloat2(label, v, v_min, v_max, format, power);
	}

	bool SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, float power) {
		return ImGui::SliderFloat3(label, v, v_min, v_max, format, power);
	}

	bool SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, float power) {
		return ImGui::SliderFloat4(label, v, v_min, v_max, format, power);
	}

	bool SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format) {
		return ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format);
	}

	bool ColorEdit3(const char* label, float col[3], DebugGui::ColorEditFlags flags) {
		return ImGui::ColorEdit3(label, col, flags);
	}

	bool ColorEdit4(const char* label, float col[4], DebugGui::ColorEditFlags flags) {
		return ImGui::ColorEdit4(label, col, flags);
	}

	bool Button(const char* label, const Vector2& size) {
		ImVec2 sizei(size._x, size._y);
		return ImGui::Button(label, sizei);
	}

	bool Combo(const char* label, s32* current_item, const char* const items[], s32 items_count, s32 popup_max_height_in_items) {
		return ImGui::Combo(label, current_item, items, items_count, popup_max_height_in_items);
	}

	bool Checkbox(const char* label, bool* v) {
		return ImGui::Checkbox(label, v);
	}

	bool BeginTabBar(const char* str_id, s32 flags) {
		return ImGui::BeginTabBar(str_id, flags);
	}

	void EndTabBar() {
		ImGui::EndTabBar();
	}
	bool BeginTabItem(const char* label, bool* p_open, s32 flags) {
		return ImGui::BeginTabItem(label, p_open, flags);
	}
	void EndTabItem() {
		ImGui::EndTabItem();
	}
	void Columns(s32 count, const char* id, bool border) {
		ImGui::Columns(count, id, border);
	}
	bool TreeNode(const char* label) {
		return ImGui::TreeNode(label);
	}
	bool TreeNode(s32 id, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		const void* ptr_id = reinterpret_cast<void*>(static_cast<s64>(id));
		bool result = ImGui::TreeNodeV(ptr_id, fmt, args);
		va_end(args);
		return result;
	}
	void TreePop() {
		ImGui::TreePop();
	}
	void NextColumn() {
		ImGui::NextColumn();
	}

	void Separator() {
		ImGui::Separator();
	}

	void SameLine(f32 offsetFromStartX, f32 spacing) {
		ImGui::SameLine(offsetFromStartX, spacing);
	}

	void SetColumnWidth(s32 columnIndex, f32 width) {
		ImGui::SetColumnWidth(columnIndex, width);
	}

	f32 GetColumnWidth(s32 columnIndex) {
		return ImGui::GetColumnWidth(columnIndex);
	}

	Vector2 GetItemInnerSpacing() {
		return Vector2(ImGui::GetStyle().ItemInnerSpacing.x, ImGui::GetStyle().ItemInnerSpacing.y);
	}

	Vector2 GetCursorScreenPos() {
		return Vector2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
	}

	void Image(GpuDescriptorHandle user_texture_id, const Vector2& size, const Vector2& uv0, const Vector2& uv1, const Color4& tint_col, const Color4& border_col, ColorChannelFilter channel, const Vector2& colorRange, TextureSmplerType samplerType) {
		ImVec2 sizei(size._x, size._y);
		ImVec2 uv0i(uv0._x, uv0._y);
		ImVec2 uv1i(uv1._x, uv1._y);
		ImVec2 color_range(colorRange._x, colorRange._y);
		ImVec4 tint_coli(tint_col._r, tint_col._g, tint_col._b, tint_col._a);
		ImVec4 border_coli(border_col._r, border_col._g, border_col._b, border_col._a);
		D3D12_GPU_DESCRIPTOR_HANDLE handle = toD3d12(user_texture_id);
		ImGui::Image(reinterpret_cast<void*>(handle.ptr), sizei, uv0i, uv1i, tint_coli, border_coli, static_cast<ImColorChannelFilter>(channel), color_range, static_cast<ImTextureSmplerType>(samplerType));
	}

	void ProgressBar(float fraction, const Vector2& sizeArg, const char* overlay) {
		ImVec2 sizeArgi(sizeArg._x, sizeArg._y);
		ImGui::ProgressBar(fraction, sizeArgi, overlay);
	}

	void AddRectFilled(const Vector2& min, const Vector2& max, const Color4& col, f32 rounding, DrawCornerFlags roundingCorners) {
		ImVec2 mini(min._x, min._y);
		ImVec2 maxi(max._x, max._y);
		ImU32 coli = ImColor(ImVec4(col._r, col._g, col._b, col._a));
		ImDrawCornerFlags flags = static_cast<ImDrawCornerFlags>(roundingCorners);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(mini, maxi, coli, rounding, flags);
	}
}