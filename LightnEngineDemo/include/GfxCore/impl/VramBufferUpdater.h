#pragma once
#include <Core/System.h>
#include <GfxCore/GfxModuleSettings.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
#include <GfxCore/impl/GpuResourceImpl.h>

struct UpdateHeader {
	GpuResource* _sourceResource = nullptr;
	GpuResource* _dstResource = nullptr;
	u32 _sourceOffset = 0;
	u32 _dstOffset = 0;
	u32 _copySizeInByte = 0;
};

struct StagingUpdateHeader {
	GpuResource* _dstResource = nullptr;
	u32 _dstOffset = 0;
	u32 _copySizeInByte = 0;
	u32 _stagingBufferOffset = 0;
};

struct TextureUpdateHeader {
	static constexpr u32 SUBRESOURCE_COUNT_MAX = 64;
	GpuTexture* _dstTexture = nullptr;
	u32 _firstSubResources = 0;
	u32 _numSubresources = 0;
	u32 _stagingBufferOffset = 0;
	SubResourceData _subResources[SUBRESOURCE_COUNT_MAX] = {};
};

class LTN_GFX_CORE_API VramBufferUpdater {
public:
	static constexpr u32 STAGING_BUFFER_SIZE_IN_BYTE = 1024 * 1024 * 512;
	static constexpr u32 BUFFER_HEADER_COUNT_MAX = 1024 * 16;
	static constexpr u32 TEXTURE_HEADER_COUNT_MAX = 512;

	void initialize();
	void terminate();
	void update();

	void enqueueUpdate(GpuResource* dstBuffer, u32 dstOffset, GpuResource* sourceBuffer, u32 sourceOffset, u32 copySizeInByte);
	void* enqueueUpdate(GpuResource* dstBuffer, u32 dstOffset, u32 copySizeInByte);
	void* enqueueUpdate(GpuTexture* dstTexture, u32 subResourceCount, const SubResourceData* subResources, u32 copySizeInByte);

	template<class T>
	T* enqueueUpdate(GpuResource* dstBuffer, u32 dstOffset, u32 numElements) {
		return reinterpret_cast<T*>(enqueueUpdate(dstBuffer, dstOffset, sizeof(T) * numElements));
	}

	template<class T>
	T* enqueueUpdate(GpuResource* dstBuffer, u32 dstOffset) {
		return reinterpret_cast<T*>(enqueueUpdate(dstBuffer, dstOffset, sizeof(T)));
	}

	void populateCommandList(CommandList* commandList);

private:
	void* allocateUpdateBuffer(u32 sizeInByte, u32 alignment);
	u32 getStagingBufferStartOffset(const void* allocatedPtr) const;

private:
	static constexpr u32 OVER_RUN_MARKER = 0xfbfbfbfbU;
	UpdateHeader _updateHeaders[BUFFER_HEADER_COUNT_MAX] = {};
	StagingUpdateHeader _stagingUpdateHeaders[BUFFER_HEADER_COUNT_MAX] = {};
	TextureUpdateHeader _textureUpdateHeaders[TEXTURE_HEADER_COUNT_MAX] = {};
	GpuBuffer _stagingBuffer;
	u8* _stagingMapPtr = nullptr;
	u32 _frameStarts[BACK_BUFFER_COUNT] = {};
	u32 _prevBufferStart = 0;
	u32 _currentBufferStart = 0;
	u32 _updateHeaderCount = 0;
	u32 _stagingUpdateHeaderCount = 0;
	u32 _textureUpdateHeaderCount = 0;
	u32 _uploadSizeInByte = 0;
};