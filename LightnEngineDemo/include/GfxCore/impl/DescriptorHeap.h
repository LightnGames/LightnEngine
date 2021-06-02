#pragma once
#include <GfxCore/GraphicsSystem.h>
#include <GfxCore/impl/GraphicsApiInterface.h>

class LTN_GFX_CORE_API DescriptorHeapAllocator :public NonCopyable {
public:
	static constexpr u32 DESCRIPTOR_COUNT_MAX = 1024;
	void initialize(const DescriptorHeapDesc& desc);
	void terminate();

	DescriptorHandle allocateDescriptors(u32 numElements);
	void discardDescriptor(DescriptorHandle& descriptorHandle);
	DescriptorHeap* getDescriptorHeap() { return _descriptorHeap; }
	u32 getIncrimentSize() const { return _incrimentSize; }

private:
	DescriptorHeap* _descriptorHeap = nullptr;
	MultiDynamicQueueBlockManager _handles;
	CpuDescriptorHandle _cpuHandleStart;
	GpuDescriptorHandle _gpuHandleStart;
	u32 _incrimentSize = 0;
};