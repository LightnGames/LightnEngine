#include <GfxCore/impl/VramBufferUpdater.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>

void VramBufferUpdater::initialize() {
	GpuDynamicBufferDesc desc = {};
	desc._device = GraphicsSystemImpl::Get()->getDevice();
	desc._sizeInByte = STAGING_BUFFER_SIZE_IN_BYTE;
	_stagingBuffer.initialize(desc);
	_stagingBuffer.setDebugName("Vram Updater Staging");
	_stagingMapPtr = _stagingBuffer.map<u8>();

	// 範囲外アクセス検知用マーカーを最初に挿入
	*reinterpret_cast<u32*>(_stagingMapPtr) = OVER_RUN_MARKER;
}

void VramBufferUpdater::terminate() {
	_stagingBuffer.terminate();
}

void VramBufferUpdater::update() {
	u32 currentFrameIndex = GraphicsSystemImpl::Get()->getFrameIndex();
	_frameStarts[currentFrameIndex] = _currentBufferStart;

	u32 prevFrameIndex = (currentFrameIndex + BACK_BUFFER_COUNT - 1) % BACK_BUFFER_COUNT;
	_prevBufferStart = _frameStarts[prevFrameIndex];
}

void VramBufferUpdater::enqueueUpdate(GpuResource* dstBuffer, u32 dstOffset, GpuResource* sourceBuffer, u32 sourceOffset, u32 copySizeInByte) {
	LTN_ASSERT(copySizeInByte > 0);
	LTN_ASSERT(dstBuffer->getSizeInByte() > sourceOffset + copySizeInByte);

	u32 headerIndex = _updateHeaderCount++; // atomic incriment
	UpdateHeader& header = _updateHeaders[headerIndex];
	header._dstResource = dstBuffer;
	header._dstOffset = dstOffset;
	header._sourceResource = sourceBuffer;
	header._sourceOffset = sourceOffset;
	header._copySizeInByte = copySizeInByte;
	LTN_ASSERT(headerIndex < BUFFER_HEADER_COUNT_MAX);
}

void* VramBufferUpdater::enqueueUpdate(GpuTexture* dstTexture, u32 numSubResource, const SubResourceData* subResources, u32 copySizeInByte) {
	LTN_ASSERT(numSubResource > 0);
	LTN_ASSERT(copySizeInByte > 0);
	u32 headerIndex = _textureUpdateHeaderCount++; // atomic incriment
	void* stagingBufferPtr = allocateUpdateBuffer(copySizeInByte, GetTextureBufferAligned(copySizeInByte));

	TextureUpdateHeader& header = _textureUpdateHeaders[headerIndex];
	header._dstTexture = dstTexture;
	header._numSubresources = numSubResource;
	header._stagingBufferOffset = getStagingBufferStartOffset(stagingBufferPtr);
	for (u32 subResourceIndex = 0; subResourceIndex < numSubResource; ++subResourceIndex) {
		SubResourceData& subResource = header._subResources[subResourceIndex];
		subResource = subResources[subResourceIndex];
		subResource._data = reinterpret_cast<const u8*>(subResource._data) + reinterpret_cast<u64>(stagingBufferPtr);
	}
	LTN_ASSERT(header._stagingBufferOffset + copySizeInByte <= _stagingBuffer.getSizeInByte());
	LTN_ASSERT(headerIndex < BUFFER_HEADER_COUNT_MAX);

	return stagingBufferPtr;
}

void* VramBufferUpdater::enqueueUpdate(GpuResource* dstBuffer, u32 dstOffset, u32 copySizeInByte) {
	LTN_ASSERT(copySizeInByte > 0);
	u32 headerIndex = _stagingUpdateHeaderCount++; // atomic incriment
	void* stagingBufferPtr = allocateUpdateBuffer(copySizeInByte, 4);

	StagingUpdateHeader& header = _stagingUpdateHeaders[headerIndex];
	header._dstResource = dstBuffer;
	header._dstOffset = dstOffset;
	header._copySizeInByte = copySizeInByte;
	header._stagingBufferOffset = getStagingBufferStartOffset(stagingBufferPtr);
	LTN_ASSERT(header._stagingBufferOffset + copySizeInByte <= _stagingBuffer.getSizeInByte());
	LTN_ASSERT(headerIndex < BUFFER_HEADER_COUNT_MAX);

	return stagingBufferPtr;
}

void VramBufferUpdater::populateCommandList(CommandList* commandList) {
	constexpr u32 BARRIER_COUNT_MAX = 128;
	constexpr u32 UNKNOWN_RESOURCE_INDEX = 0xffffffff;
	GpuResource* uniqueResources[BARRIER_COUNT_MAX] = {};
	ResourceTransitionBarrier barriers[BARRIER_COUNT_MAX] = {};
	u32 barrierCount = 0;

	// 重複リソース検索ラムダ
	auto findUniqueResource = [&uniqueResources, &barrierCount, UNKNOWN_RESOURCE_INDEX](GpuResource* resource) {
		for (u32 resourceIndex = 0; resourceIndex < barrierCount; ++resourceIndex) {
			if (uniqueResources[resourceIndex] == resource) {
				return resourceIndex;
			}
		}

		return UNKNOWN_RESOURCE_INDEX;
	};

	DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::DEEP_GREEN, "Vram Updater");

	// vram to vram
	for (u32 headerIndex = 0; headerIndex < _updateHeaderCount; ++headerIndex) {
		UpdateHeader& header = _updateHeaders[headerIndex];
		u32 dstResourceIndex = findUniqueResource(header._dstResource);
		if (dstResourceIndex == UNKNOWN_RESOURCE_INDEX) {
			barriers[barrierCount] = header._dstResource->getTransitionBarrier(RESOURCE_STATE_COPY_DEST);
			uniqueResources[barrierCount++] = header._dstResource;
		}

		u32 sourceResourceIndex = findUniqueResource(header._sourceResource);
		if (sourceResourceIndex == UNKNOWN_RESOURCE_INDEX) {
			barriers[barrierCount] = header._dstResource->getTransitionBarrier(RESOURCE_STATE_COPY_SOURCE);
			uniqueResources[barrierCount++] = header._dstResource;
		}
	}

	// staging to vram
	for (u32 headerIndex = 0; headerIndex < _stagingUpdateHeaderCount; ++headerIndex) {
		StagingUpdateHeader& header = _stagingUpdateHeaders[headerIndex];
		u32 dstResourceIndex = findUniqueResource(header._dstResource);
		if (dstResourceIndex == UNKNOWN_RESOURCE_INDEX) {
			barriers[barrierCount] = header._dstResource->getTransitionBarrier(RESOURCE_STATE_COPY_DEST);
			uniqueResources[barrierCount++] = header._dstResource;
		}
	}

	// texture to vram
	for (u32 headerIndex = 0; headerIndex < _textureUpdateHeaderCount; ++headerIndex) {
		TextureUpdateHeader& header = _textureUpdateHeaders[headerIndex];
		u32 dstResourceIndex = findUniqueResource(header._dstTexture);
		if (dstResourceIndex == UNKNOWN_RESOURCE_INDEX) {
			barriers[barrierCount] = header._dstTexture->getTransitionBarrier(RESOURCE_STATE_COPY_DEST);
			uniqueResources[barrierCount++] = header._dstTexture;
		}
	}

	// コピー可能リソースステートに変更
	if (barrierCount > 0) {
		commandList->transitionBarriers(barriers, barrierCount);
	}

	// Vram to vram
	for (u32 headerIndex = 0; headerIndex < _updateHeaderCount; ++headerIndex) {
		UpdateHeader& header = _updateHeaders[headerIndex];
		commandList->copyBufferRegion(header._dstResource->getResource(), header._dstOffset, header._sourceResource->getResource(), header._sourceOffset, header._copySizeInByte);
	}

	// Staging to vram
	for (u32 headerIndex = 0; headerIndex < _stagingUpdateHeaderCount; ++headerIndex) {
		StagingUpdateHeader& header = _stagingUpdateHeaders[headerIndex];
		commandList->copyBufferRegion(header._dstResource->getResource(), header._dstOffset, _stagingBuffer.getResource(), header._stagingBufferOffset, header._copySizeInByte);
	}

	// Texture to vram
	for (u32 headerIndex = 0; headerIndex < _textureUpdateHeaderCount; ++headerIndex) {
		TextureUpdateHeader& header = _textureUpdateHeaders[headerIndex];
		Device* device = GraphicsSystemImpl::Get()->getDevice();
		ResourceDesc textureDesc = header._dstTexture->getResourceDesc();
		u64 requiredSize = 0;
		PlacedSubresourceFootprint layouts[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX];
		u32 numRows[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX];
		u64 rowSizesInBytes[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX];
		device->getCopyableFootprints(&textureDesc, header._firstSubResources, header._numSubresources, header._stagingBufferOffset, layouts, numRows, rowSizesInBytes, &requiredSize);

		for (u32 subResourceIndex = 0; subResourceIndex < header._numSubresources; ++subResourceIndex) {
			TextureCopyLocation src = {};
			src._resource = _stagingBuffer.getResource();
			src._placedFootprint = layouts[subResourceIndex];
			src._type = TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

			TextureCopyLocation dst = {};
			dst._resource = header._dstTexture->getResource();
			dst._subresourceIndex = subResourceIndex;
			dst._type = TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			commandList->copyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
	}

	// 変更したリソースを元に戻すバリアを構築
	for (u32 barrierIndex = 0; barrierIndex < barrierCount; ++barrierIndex) {
		ResourceTransitionBarrier& barrier = barriers[barrierIndex];
		ResourceStates beforeState = barrier._stateBefore;
		barrier._stateBefore = barrier._stateAfter;
		barrier._stateAfter = beforeState;
	}

	// コピー可能ステートから元のステートに戻す
	if (barrierCount > 0) {
		commandList->transitionBarriers(barriers, barrierCount);
	}

	_uploadSizeInByte = 0;
	_updateHeaderCount = 0;
	_stagingUpdateHeaderCount = 0;
	_textureUpdateHeaderCount = 0;
}

void* VramBufferUpdater::allocateUpdateBuffer(u32 sizeInByte, u32 alignment) {
	u8* allocatedPtr = nullptr;
	u32 alignedCurrentBufferStart = GetAligned(_currentBufferStart, alignment);
	if (_currentBufferStart % alignment == 0) {
		alignedCurrentBufferStart = _currentBufferStart;
	}

	if (alignedCurrentBufferStart < _prevBufferStart) {
		LTN_ASSERT(alignedCurrentBufferStart + sizeInByte < _prevBufferStart);
	}

	if (alignedCurrentBufferStart + sizeInByte <= STAGING_BUFFER_SIZE_IN_BYTE) {
		allocatedPtr = _stagingMapPtr + alignedCurrentBufferStart;
		_currentBufferStart = alignedCurrentBufferStart + sizeInByte;
		_uploadSizeInByte += sizeInByte;
		return allocatedPtr;
	}

	// バッファの端まで使っていたら先頭に戻る
	if (sizeInByte < _prevBufferStart) {
		allocatedPtr = _stagingMapPtr;
		_currentBufferStart = sizeInByte;
		_uploadSizeInByte += sizeInByte;
		return allocatedPtr;
	}

	// メモリが確保できない
	return nullptr;
}

u32 VramBufferUpdater::getStagingBufferStartOffset(const void* allocatedPtr) const {
	LTN_ASSERT(allocatedPtr != nullptr);
	const u8* ptr = reinterpret_cast<const u8*>(allocatedPtr);
	return static_cast<u32>(ptr - _stagingMapPtr);
}
