#pragma once
#include <GfxCore/GraphicsSystem.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
#include <GfxCore/impl/GpuResourceImpl.h>
#include <GfxCore/impl/DescriptorHeap.h>
#include <GfxCore/impl/DebugWindow.h>
#include <GfxCore/impl/VramBufferUpdater.h>
#include <functional>

using RenderPass = std::function<void(CommandList*)>;
template class LTN_GFX_CORE_API std::function<void(CommandList*)>;

class LTN_GFX_CORE_API GraphicsSystemImpl :public GraphicsSystem {
public:
	void initialize();
	void update();
	void processDeletion();
	void render();
	void terminate();

	DescriptorHeapAllocator* getDsvGpuDescriptorAllocator() { return  &_dsvDescriptorAllocator; }
	DescriptorHeapAllocator* getRtvGpuDescriptorAllocator() { return  &_rtvDescriptorAllocator; }
	DescriptorHeapAllocator* getSrvCbvUavCpuDescriptorAllocator() { return  &_srvCbvUavCpuDescriptorAllocator; }
	DescriptorHeapAllocator* getSrvCbvUavGpuDescriptorAllocator() { return  &_srvCbvUavGpuDescriptorAllocator; }
	VramBufferUpdater* getVramUpdater() { return &_vramBufferUpdater; }
	Device* getDevice() { return _device; }
	HardwareAdapter* getHardWareAdaptor() { return _adapter; }
	u32 getFrameIndex() const { return _frameIndex; }
	void setRenderPass(RenderPass pass) { _renderPass = pass; }
	bool isInitialized() const;
	void beginDebugWindow();

	static GraphicsSystemImpl* Get();

private:
	void moveToNextFrame();

private:
	Device* _device = nullptr;
	HardwareFactory* _factory = nullptr;
	HardwareAdapter* _adapter = nullptr;
	SwapChain* _swapChain = nullptr;
	CommandQueue* _commandQueue = nullptr;

	DescriptorHeapAllocator _srvCbvUavCpuDescriptorAllocator;
	DescriptorHeapAllocator _srvCbvUavGpuDescriptorAllocator;
	DescriptorHeapAllocator _rtvDescriptorAllocator;
	DescriptorHeapAllocator _dsvDescriptorAllocator;
	VramBufferUpdater _vramBufferUpdater;
	DebugWindow _debugWindow;
	u64 _fenceValues[BACK_BUFFER_COUNT] = {};
	GpuTexture _backBuffers[BACK_BUFFER_COUNT] = {};
	u32 _frameIndex = 0;
	RenderPass _renderPass;
};