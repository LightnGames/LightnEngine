#include <GfxCore/impl/DescriptorHeap.h>

void DescriptorHeapAllocator::initialize(const DescriptorHeapDesc& desc) {
	_descriptorHeap = GraphicsApiInstanceAllocator::Get()->allocateDescriptorHeap();
	_descriptorHeap->initialize(desc);

	_incrimentSize = desc._device->getDescriptorHandleIncrementSize(desc._type);
	_cpuHandleStart = _descriptorHeap->getCPUDescriptorHandleForHeapStart();
	_gpuHandleStart = _descriptorHeap->getGPUDescriptorHandleForHeapStart();
	_handles.initialize(desc._numDescriptors);
}

void DescriptorHeapAllocator::terminate() {
	LTN_ASSERT(_handles.isEmpty());
	_descriptorHeap->terminate();
	_handles.terminate();
}

DescriptorHandle DescriptorHeapAllocator::allocateDescriptors(u32 numElements) {
	u64 offset = _handles.request(numElements) * static_cast<u64>(_incrimentSize);
	DescriptorHandle handle;
	handle._cpuHandle = _cpuHandleStart + offset;
	handle._gpuHandle = _gpuHandleStart + offset;
	handle._numDescriptor = numElements;
	return handle;
}

void DescriptorHeapAllocator::discardDescriptor(DescriptorHandle& descriptorHandle) {
	u64 index = (descriptorHandle._cpuHandle._ptr - _cpuHandleStart._ptr) / _incrimentSize;
	LTN_ASSERT(index < DESCRIPTOR_COUNT_MAX);
	_handles.discard(static_cast<u32>(index), descriptorHandle._numDescriptor);
}
