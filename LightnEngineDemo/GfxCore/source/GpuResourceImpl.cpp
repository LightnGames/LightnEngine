#include <GfxCore/impl/GpuResourceImpl.h>

void GpuResource::initialize() {
	_resource = GraphicsApiInstanceAllocator::Get()->allocateResource();
}

void GpuResource::terminate() {
	_resource->terminate();
}

void GpuResource::transitionResource(CommandList* commandList, ResourceStates stateAfter) {
	commandList->transitionBarrierSimple(getResource(), _currentState, stateAfter);
	_currentState = stateAfter;
}

void GpuResource::setDebugName(const char* name) {
	_resource->setDebugName(name);
}

ResourceTransitionBarrier GpuResource::getTransitionBarrier(ResourceStates stateAfter) {
	LTN_ASSERT(_resource != nullptr);
	ResourceTransitionBarrier barrier = {};
	barrier._resource = _resource;
	barrier._stateAfter = stateAfter;
	barrier._stateBefore = _currentState;
	return barrier;
}

ResourceTransitionBarrier GpuResource::getAndUpdateTransitionBarrier(ResourceStates stateAfter) {
	ResourceTransitionBarrier barrier = getTransitionBarrier(stateAfter);
	_currentState = stateAfter;
	return barrier;
}

ConstantBufferViewDesc GpuResource::getConstantBufferViewDesc() const {
	ConstantBufferViewDesc desc = {};
	desc._bufferLocation = getGpuVirtualAddress();
	desc._sizeInBytes = getSizeInByte();
	return desc;
}

void GpuTexture::initializeFromBackbuffer(SwapChain* swapChain, u32 backBufferIndex) {
	GpuResource::initialize();
	swapChain->getBackBuffer(&_resource, backBufferIndex);
	_currentState = RESOURCE_STATE_PRESENT;
}

void GpuTexture::initialize(const GpuTextureDesc& desc) {
	GpuResource::initialize();
	ResourceDesc textureDesc = {};
	textureDesc._format = desc._format;
	textureDesc._dimension = RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc._width = desc._width;
	textureDesc._height = desc._height;
	textureDesc._depthOrArraySize = desc._arraySize;
	textureDesc._mipLevels = desc._mipLevels;
	textureDesc._sampleDesc._count = desc._sampleCount;
	textureDesc._sampleDesc._quality = desc._sampleQuality;
	textureDesc._flags = desc._flags;
	desc._device->createCommittedResource(HEAP_TYPE_DEFAULT, HEAP_FLAG_NONE, textureDesc, desc._initialState, desc._optimizedClearValue, _resource);
	_currentState = desc._initialState;
	_desc = textureDesc;
}

void GpuBuffer::initialize(const GpuBufferDesc& desc) {
	GpuResource::initialize();

	ResourceDesc bufferDesc = {};
	bufferDesc._dimension = RESOURCE_DIMENSION_BUFFER;
	bufferDesc._width = desc._sizeInByte;
	bufferDesc._height = 1;
	bufferDesc._depthOrArraySize = 1;
	bufferDesc._mipLevels = 1;
	bufferDesc._sampleDesc._count = 1;
	bufferDesc._flags = desc._flags;
	bufferDesc._layout = TEXTURE_LAYOUT_ROW_MAJOR;
	desc._device->createCommittedResource(desc._heapType, HEAP_FLAG_NONE, bufferDesc, desc._initialState, nullptr, _resource);
	_sizeInByte = desc._sizeInByte;
	_currentState = desc._initialState;
	_gpuVirtualAddress = _resource->getGpuVirtualAddress();
}

void GpuBuffer::initialize(const GpuDynamicBufferDesc& desc) {
	GpuResource::initialize();

	ResourceDesc bufferDesc = {};
	bufferDesc._dimension = RESOURCE_DIMENSION_BUFFER;
	bufferDesc._width = desc._sizeInByte;
	bufferDesc._height = 1;
	bufferDesc._depthOrArraySize = 1;
	bufferDesc._mipLevels = 1;
	bufferDesc._sampleDesc._count = 1;
	bufferDesc._layout = TEXTURE_LAYOUT_ROW_MAJOR;
	desc._device->createCommittedResource(HEAP_TYPE_UPLOAD, HEAP_FLAG_NONE, bufferDesc, RESOURCE_STATE_GENERIC_READ, nullptr, _resource);
	_sizeInByte = desc._sizeInByte;
	_currentState = RESOURCE_STATE_GENERIC_READ;
	_gpuVirtualAddress = _resource->getGpuVirtualAddress();
}

void GpuBuffer::terminate() {
	GpuResource::terminate();
}

void Shader::initialize(const char* filePath) {
	_blob = GraphicsApiInstanceAllocator::Get()->allocateShaderBlob();
	_blob->initialize(filePath);
}

void Shader::terminate() {
	_blob->terminate();
	_blob = nullptr;
}

ShaderByteCode Shader::getShaderByteCode() const {
	return _blob->getShaderByteCode();
}
