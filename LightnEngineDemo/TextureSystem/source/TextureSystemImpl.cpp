#include <TextureSystem/impl/TextureSystemImpl.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/VramBufferUpdater.h>

TextureSystemImpl _textureSystem;
void TextureSystemImpl::initialize() {
    GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
    DescriptorHeapAllocator* allocater = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator();
    _textureAssets = new Asset[TEXTURE_COUNT_MAX];
	_textures.initialize(TEXTURE_COUNT_MAX);
	_descriptors = allocater->allocateDescriptors(TEXTURE_COUNT_MAX);

    TextureDesc desc = {};
    desc._filePath = "common/common_black.dds";
    _commonBlackTexture = allocateTexture(desc);
    _commonBlackTexture->getAsset()->requestLoad();
    loadTexture(0);
    Resource* commonBlackResource = static_cast<TextureImpl*>(_commonBlackTexture)->getGpuTexture()->getResource();

    // デスクリプター初期値として黒のテクスチャで埋める
    Device* device = graphicsSystem->getDevice();
    u32 incrimentSize = allocater->getIncrimentSize();
    for (u32 textureIndex = 0; textureIndex < TEXTURE_COUNT_MAX; ++textureIndex) {
        CpuDescriptorHandle descriptor = _descriptors._cpuHandle + static_cast<u64>(incrimentSize) * textureIndex;
        device->createShaderResourceView(commonBlackResource, nullptr, descriptor);
    }
}

void TextureSystemImpl::update() {
    u32 textureCount = _textures.getInstanceCount();
    for (u32 textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
        if (_assetStateFlags[textureIndex] & ASSET_STATE_REQUEST_LOAD) {
            loadTexture(textureIndex);
        }
    }

    debugDrawGui();
}

void TextureSystemImpl::processDeletion() {
	u32 textureCount = _textures.getInstanceCount();
	for (u32 textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
		if (_stateFlags[textureIndex] & TEXTURE_STATE_REQUEST_DELETE) {
			_textures[textureIndex].terminate();
			_textures.discard(textureIndex);
			_stateFlags[textureIndex] = TEXTURE_STATE_NONE;
			_fileHashes[textureIndex] = 0;
		}
	}
}

void TextureSystemImpl::terminate() {
    _commonBlackTexture->requestToDelete();
	processDeletion();
	LTN_ASSERT(_textures.getInstanceCount() == 0);
	_textures.terminate();
    delete[] _textureAssets;
	GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator()->discardDescriptor(_descriptors);
}

void TextureSystemImpl::debugDrawGui() {
	DebugWindow::StartWindow("Textures");
    DebugGui::Columns(1, nullptr, true);
	u32 incrimentSize = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator()->getIncrimentSize();
	u32 textureCount = _textures.getInstanceCount();
	for (u32 textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
		if (_fileHashes[textureIndex] == 0) {
			continue;
		}
        ResourceDesc desc = _textures[textureIndex].getGpuTexture()->getResourceDesc();

        DebugGui::Text("[%-2d] %-s", textureIndex, _debugNames[textureIndex]._name);
        DebugGui::Text("width:%-4d height:%-4d", desc._width, desc._height);
        DebugGui::Image(_descriptors._gpuHandle + incrimentSize * textureIndex, Vector2(128, 128));
        DebugGui::NextColumn();
	}
    DebugGui::Columns(1);
	DebugWindow::End();
}

Texture* TextureSystemImpl::createTexture(const TextureDesc& desc) {
    GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
    u64 incrimentSize = static_cast<u64>(graphicsSystem->getSrvCbvUavGpuDescriptorAllocator()->getIncrimentSize());
    u64 fileHash = StrHash(desc._filePath);
	u32 textureIndex = findTextureFileHash(fileHash);
	if (textureIndex != static_cast<u32>(-1)) {
        return &_textures[textureIndex];
	}

    Texture* texture = allocateTexture(desc);
    texture->getAsset()->requestLoad();
	return texture;
}

Texture* TextureSystemImpl::allocateTexture(const TextureDesc& desc) {
    u64 fileHash = StrHash(desc._filePath);
    u32 textureIndex = findTextureFileHash(fileHash);
    if (textureIndex == static_cast<u32>(-1)) {
        textureIndex = _textures.request();

        // アセット実パスに変換
        char textureFilePath[FILE_PATH_COUNT_MAX] = {};
        sprintf_s(textureFilePath, "%s/%s", RESOURCE_FOLDER_PATH, desc._filePath);

        TextureImpl& texture = _textures[textureIndex];
        Asset* textureAsset = &_textureAssets[textureIndex];
        textureAsset->setAssetStateFlags(&_assetStateFlags[textureIndex]);
        textureAsset->openFile(textureFilePath);
        textureAsset->closeFile();

        texture.setGpuTexture(&_gpuTextures[textureIndex]);
        texture.setAsset(&_textureAssets[textureIndex]);
        texture.setStateFlags(&_stateFlags[textureIndex]);
        _fileHashes[textureIndex] = fileHash;
        _assetStateFlags[textureIndex] = ASSET_STATE_ALLOCATED;

        TextureDebugName& debugName = _debugNames[textureIndex];
        memcpy(debugName._name, desc._filePath, StrLength(desc._filePath));

        if (_commonBlackTexture != nullptr) {
            GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
            DescriptorHeapAllocator* allocater = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator();
            Device* device = graphicsSystem->getDevice();
            u32 incrimentSize = allocater->getIncrimentSize();
            CpuDescriptorHandle descriptor = _descriptors._cpuHandle + static_cast<u64>(incrimentSize) * textureIndex;
            Resource* commonBlackResource = static_cast<TextureImpl*>(_commonBlackTexture)->getGpuTexture()->getResource();
            device->createShaderResourceView(commonBlackResource, nullptr, descriptor);
        }
    }

    TextureImpl* texture = &_textures[textureIndex];
    return texture;
}

u32 TextureSystemImpl::getTextureIndex(const Texture* texture) const {
    return _textures.getArrayIndex(static_cast<const TextureImpl*>(texture));
}

Texture* TextureSystemImpl::findTexture(u64 fileHash) {
    u32 textureCount = _textures.getResarveCount();
    for (u32 textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
        if (_fileHashes[textureIndex] == fileHash) {
            return &_textures[textureIndex];
        }
    }
    return nullptr;
}

TextureSystem* TextureSystem::Get() {
	return &_textureSystem;
}

TextureSystemImpl* TextureSystemImpl::Get() {
	return &_textureSystem;
}

u32 TextureSystemImpl::findTextureFileHash(u64 hash) {
	u32 textureCount = _textures.getResarveCount();
	for (u32 textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
		if (_fileHashes[textureIndex] == hash) {
			return textureIndex;
		}
	}

	return static_cast<u32>(-1);
}

size_t BitsPerPixel(Format fmt) noexcept {
    switch (fmt) {
    case FORMAT_R32G32B32A32_TYPELESS:
    case FORMAT_R32G32B32A32_FLOAT:
    case FORMAT_R32G32B32A32_UINT:
    case FORMAT_R32G32B32A32_SINT:
        return 128;

    case FORMAT_R32G32B32_TYPELESS:
    case FORMAT_R32G32B32_FLOAT:
    case FORMAT_R32G32B32_UINT:
    case FORMAT_R32G32B32_SINT:
        return 96;

    case FORMAT_R16G16B16A16_TYPELESS:
    case FORMAT_R16G16B16A16_FLOAT:
    case FORMAT_R16G16B16A16_UNORM:
    case FORMAT_R16G16B16A16_UINT:
    case FORMAT_R16G16B16A16_SNORM:
    case FORMAT_R16G16B16A16_SINT:
    case FORMAT_R32G32_TYPELESS:
    case FORMAT_R32G32_FLOAT:
    case FORMAT_R32G32_UINT:
    case FORMAT_R32G32_SINT:
    case FORMAT_R32G8X24_TYPELESS:
    case FORMAT_D32_FLOAT_S8X24_UINT:
    case FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case FORMAT_X32_TYPELESS_G8X24_UINT:
    case FORMAT_Y416:
    case FORMAT_Y210:
    case FORMAT_Y216:
        return 64;

    case FORMAT_R10G10B10A2_TYPELESS:
    case FORMAT_R10G10B10A2_UNORM:
    case FORMAT_R10G10B10A2_UINT:
    case FORMAT_R11G11B10_FLOAT:
    case FORMAT_R8G8B8A8_TYPELESS:
    case FORMAT_R8G8B8A8_UNORM:
    case FORMAT_R8G8B8A8_UNORM_SRGB:
    case FORMAT_R8G8B8A8_UINT:
    case FORMAT_R8G8B8A8_SNORM:
    case FORMAT_R8G8B8A8_SINT:
    case FORMAT_R16G16_TYPELESS:
    case FORMAT_R16G16_FLOAT:
    case FORMAT_R16G16_UNORM:
    case FORMAT_R16G16_UINT:
    case FORMAT_R16G16_SNORM:
    case FORMAT_R16G16_SINT:
    case FORMAT_R32_TYPELESS:
    case FORMAT_D32_FLOAT:
    case FORMAT_R32_FLOAT:
    case FORMAT_R32_UINT:
    case FORMAT_R32_SINT:
    case FORMAT_R24G8_TYPELESS:
    case FORMAT_D24_UNORM_S8_UINT:
    case FORMAT_R24_UNORM_X8_TYPELESS:
    case FORMAT_X24_TYPELESS_G8_UINT:
    case FORMAT_R9G9B9E5_SHAREDEXP:
    case FORMAT_R8G8_B8G8_UNORM:
    case FORMAT_G8R8_G8B8_UNORM:
    case FORMAT_B8G8R8A8_UNORM:
    case FORMAT_B8G8R8X8_UNORM:
    case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case FORMAT_B8G8R8A8_TYPELESS:
    case FORMAT_B8G8R8A8_UNORM_SRGB:
    case FORMAT_B8G8R8X8_TYPELESS:
    case FORMAT_B8G8R8X8_UNORM_SRGB:
    case FORMAT_AYUV:
    case FORMAT_Y410:
    case FORMAT_YUY2:
        return 32;

    case FORMAT_P010:
    case FORMAT_P016:
    case FORMAT_V408:
        return 24;

    case FORMAT_R8G8_TYPELESS:
    case FORMAT_R8G8_UNORM:
    case FORMAT_R8G8_UINT:
    case FORMAT_R8G8_SNORM:
    case FORMAT_R8G8_SINT:
    case FORMAT_R16_TYPELESS:
    case FORMAT_R16_FLOAT:
    case FORMAT_D16_UNORM:
    case FORMAT_R16_UNORM:
    case FORMAT_R16_UINT:
    case FORMAT_R16_SNORM:
    case FORMAT_R16_SINT:
    case FORMAT_B5G6R5_UNORM:
    case FORMAT_B5G5R5A1_UNORM:
    case FORMAT_A8P8:
    case FORMAT_B4G4R4A4_UNORM:
    case FORMAT_P208:
    case FORMAT_V208:
        return 16;

    case FORMAT_NV12:
    case FORMAT_420_OPAQUE:
    case FORMAT_NV11:
        return 12;

    case FORMAT_R8_TYPELESS:
    case FORMAT_R8_UNORM:
    case FORMAT_R8_UINT:
    case FORMAT_R8_SNORM:
    case FORMAT_R8_SINT:
    case FORMAT_A8_UNORM:
    case FORMAT_AI44:
    case FORMAT_IA44:
    case FORMAT_P8:
        return 8;

    case FORMAT_R1_UNORM:
        return 1;

    case FORMAT_BC1_TYPELESS:
    case FORMAT_BC1_UNORM:
    case FORMAT_BC1_UNORM_SRGB:
    case FORMAT_BC4_TYPELESS:
    case FORMAT_BC4_UNORM:
    case FORMAT_BC4_SNORM:
        return 4;

    case FORMAT_BC2_TYPELESS:
    case FORMAT_BC2_UNORM:
    case FORMAT_BC2_UNORM_SRGB:
    case FORMAT_BC3_TYPELESS:
    case FORMAT_BC3_UNORM:
    case FORMAT_BC3_UNORM_SRGB:
    case FORMAT_BC5_TYPELESS:
    case FORMAT_BC5_UNORM:
    case FORMAT_BC5_SNORM:
    case FORMAT_BC6H_TYPELESS:
    case FORMAT_BC6H_UF16:
    case FORMAT_BC6H_SF16:
    case FORMAT_BC7_TYPELESS:
    case FORMAT_BC7_UNORM:
    case FORMAT_BC7_UNORM_SRGB:
        return 8;

    default:
        return 0;
    }
}

HRESULT GetSurfaceInfo(
    u32 width,
    u32 height,
    Format fmt,
    u32* outNumBytes,
    u32* outRowBytes,
    u32* outNumRows) noexcept {
    uint64_t numBytes = 0;
    uint64_t rowBytes = 0;
    uint64_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;
    switch (fmt) {
    case FORMAT_BC1_TYPELESS:
    case FORMAT_BC1_UNORM:
    case FORMAT_BC1_UNORM_SRGB:
    case FORMAT_BC4_TYPELESS:
    case FORMAT_BC4_UNORM:
    case FORMAT_BC4_SNORM:
        bc = true;
        bpe = 8;
        break;

    case FORMAT_BC2_TYPELESS:
    case FORMAT_BC2_UNORM:
    case FORMAT_BC2_UNORM_SRGB:
    case FORMAT_BC3_TYPELESS:
    case FORMAT_BC3_UNORM:
    case FORMAT_BC3_UNORM_SRGB:
    case FORMAT_BC5_TYPELESS:
    case FORMAT_BC5_UNORM:
    case FORMAT_BC5_SNORM:
    case FORMAT_BC6H_TYPELESS:
    case FORMAT_BC6H_UF16:
    case FORMAT_BC6H_SF16:
    case FORMAT_BC7_TYPELESS:
    case FORMAT_BC7_UNORM:
    case FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bpe = 16;
        break;

    case FORMAT_R8G8_B8G8_UNORM:
    case FORMAT_G8R8_G8B8_UNORM:
    case FORMAT_YUY2:
        packed = true;
        bpe = 4;
        break;

    case FORMAT_Y210:
    case FORMAT_Y216:
        packed = true;
        bpe = 8;
        break;

    case FORMAT_NV12:
    case FORMAT_420_OPAQUE:
    case FORMAT_P208:
        planar = true;
        bpe = 2;
        break;

    case FORMAT_P010:
    case FORMAT_P016:
        planar = true;
        bpe = 4;
        break;

    default:
        break;
    }

    if (bc) {
        uint64_t numBlocksWide = 0;
        if (width > 0) {
            numBlocksWide = Max(1u, (width + 3u) / 4u);
        }
        uint64_t numBlocksHigh = 0;
        if (height > 0) {
            numBlocksHigh = Max(1u, (height + 3u) / 4u);
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    }
    else if (packed) {
        rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
        numRows = uint64_t(height);
        numBytes = rowBytes * height;
    }
    else if (fmt == FORMAT_NV11) {
        rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
        numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    }
    else if (planar) {
        rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
        numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
        numRows = height + ((uint64_t(height) + 1u) >> 1);
    }
    else {
        size_t bpp = BitsPerPixel(fmt);
        if (!bpp)
            return E_INVALIDARG;

        rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
        numRows = uint64_t(height);
        numBytes = rowBytes * height;
    }

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
    static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
    if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
#else
    static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

    if (outNumBytes) {
        *outNumBytes = static_cast<u32>(numBytes);
    }
    if (outRowBytes) {
        *outRowBytes = static_cast<u32>(rowBytes);
    }
    if (outNumRows) {
        *outNumRows = static_cast<u32>(numRows);
    }

    return S_OK;
}

void TextureImpl::initialize(const TextureDesc& desc) {
}

void TextureSystemImpl::loadTexture(u32 textureIndex) {
    LTN_ASSERT(_assetStateFlags[textureIndex] == ASSET_STATE_REQUEST_LOAD);

    GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
    u64 incrimentSize = static_cast<u64>(graphicsSystem->getSrvCbvUavGpuDescriptorAllocator()->getIncrimentSize());

    Device* device = GraphicsSystemImpl::Get()->getDevice();
    GpuTexture& gpuTexture = _gpuTextures[textureIndex];

    Asset* asset = _textures[textureIndex].getAsset();
    asset->openFile(asset->getFilePath());
    FILE* fin = asset->getFile();

    struct DdsPixelFormat {
        u32    _size;
        u32    _flags;
        u32    _fourCC;
        u32    _RGBBitCount;
        u32    _RBitMask;
        u32    _GBitMask;
        u32    _BBitMask;
        u32    _ABitMask;
    };

    struct DdsHeader {
        u32        _size;
        u32        _flags;
        u32        _height;
        u32        _width;
        u32        _pitchOrLinearSize;
        u32        _depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
        u32        _mipMapCount;
        u32        _reserved1[11];
        DdsPixelFormat _ddspf;
        u32        _caps;
        u32        _caps2;
        u32        _caps3;
        u32        _caps4;
        u32        _reserved2;
    };

    struct DdsHeaderDxt10 {
        Format     _dxgiFormat;
        u32        _resourceDimension;
        u32        _miscFlag; // see D3D11_RESOURCE_MISC_FLAG
        u32        _arraySize;
        u32        _miscFlags2;
    };

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                          \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
    const u32 DDS_MAGIC = 0x20534444; // "DDS "
    u32 fileMagic = 0;
    fread_s(&fileMagic, sizeof(u32), sizeof(u32), 1, fin);
    LTN_ASSERT(fileMagic == DDS_MAGIC);

    DdsHeader ddsHeader;
    fread_s(&ddsHeader, sizeof(DdsHeader), sizeof(DdsHeader), 1, fin);
    LTN_ASSERT(ddsHeader._ddspf._flags & DDS_FOURCC);
    LTN_ASSERT(MAKEFOURCC('D', 'X', '1', '0') == ddsHeader._ddspf._fourCC);

    DdsHeaderDxt10 ddsHeaderDxt10;
    fread_s(&ddsHeaderDxt10, sizeof(DdsHeaderDxt10), sizeof(DdsHeaderDxt10), 1, fin);

    u32 mipCount = Max(ddsHeader._mipMapCount, 1U);
    u32 numberOfPlanes = device->getFormatPlaneCount(ddsHeaderDxt10._dxgiFormat);
    bool is3dTexture = ddsHeaderDxt10._resourceDimension == RESOURCE_DIMENSION_TEXTURE3D;
    bool isCubeMapTexture = false;

    u32 numberOfResources = is3dTexture ? 1 : ddsHeaderDxt10._arraySize;
    numberOfResources *= mipCount;
    numberOfResources *= numberOfPlanes;
    LTN_ASSERT(numberOfResources <= TextureUpdateHeader::SUBRESOURCE_COUNT_MAX);
    SubResourceData subResources[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX] = {};

    // gpu resources
    GpuTextureDesc resourceDesc = {};
    resourceDesc._device = device;
    resourceDesc._format = ddsHeaderDxt10._dxgiFormat;
    resourceDesc._width = ddsHeader._width;
    resourceDesc._height = ddsHeader._height;
    resourceDesc._mipLevels = mipCount;
    resourceDesc._initialState = RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    gpuTexture.initialize(resourceDesc);
    gpuTexture.setDebugName(_debugNames[textureIndex]._name);

    ResourceDesc textureDesc = gpuTexture.getResourceDesc();
    u64 requiredSize = 0;
    PlacedSubresourceFootprint layouts[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX] = {};
    u32 numRows[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX] = {};
    u64 rowSizesInBytes[TextureUpdateHeader::SUBRESOURCE_COUNT_MAX] = {};
    device->getCopyableFootprints(&textureDesc, 0, numberOfResources, 0, layouts, numRows, rowSizesInBytes, &requiredSize);

    u32 subResourceIndex = 0;
    for (u32 planeIndex = 0; planeIndex < numberOfPlanes; ++planeIndex) {
        u64 srcBits = 0;
        for (u32 arrayIndex = 0; arrayIndex < ddsHeaderDxt10._arraySize; arrayIndex++) {
            u32 w = ddsHeader._width;
            u32 h = ddsHeader._width;
            u32 d = ddsHeader._depth;
            for (u32 mipIndex = 0; mipIndex < mipCount; mipIndex++) {
                u32 numBytes = 0;
                u32 rowBytes = 0;
                LTN_SUCCEEDED(GetSurfaceInfo(w, h, ddsHeaderDxt10._dxgiFormat, &numBytes, &rowBytes, nullptr));

                SubResourceData& subResource = subResources[subResourceIndex++];
                subResource._data = reinterpret_cast<void*>(srcBits);
                subResource._rowPitch = static_cast<s64>(rowBytes);
                subResource._slicePitch = static_cast<s64>(numBytes);

                // AdjustPlaneResource(ddsHeaderDxt10._dxgiFormat, h, planeIndex, subResource);
                srcBits += numBytes * d;

                w = Max(w / 2, 1u);
                h = Max(h / 2, 1u);
                d = Max(d / 2, 1u);
            }
        }
    }

    VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
    void* pixelData = vramUpdater->enqueueUpdate(&gpuTexture, numberOfResources, subResources, static_cast<u32>(requiredSize));

    for (u32 subResourceIndex = 0; subResourceIndex < numberOfResources; ++subResourceIndex) {
        const PlacedSubresourceFootprint& layout = layouts[subResourceIndex];
        u8* data = reinterpret_cast<u8*>(pixelData) + layout._offset;
        u64 rowSizeInBytes = rowSizesInBytes[subResourceIndex];
        u32 numRow = numRows[subResourceIndex];
        u32 rowPitch = layout._footprint._rowPitch;
        u32 slicePitch = layout._footprint._rowPitch * numRow;
        u32 numSlices = layout._footprint._depth;
        for (u32 z = 0; z < numSlices; ++z) {
            u64 slicePitchOffset = static_cast<u64>(slicePitch) * z;
            u8* destSlice = data + slicePitchOffset;
            for (u32 y = 0; y < numRow; ++y) {
                u64 rowPitchOffset = static_cast<u64>(rowPitch) * y;
                fread_s(destSlice + rowPitchOffset, sizeof(u8)* rowSizeInBytes, sizeof(u8), rowSizeInBytes, fin);
            }
        }
    }

    _textureAssets[textureIndex].closeFile();
    _assetStateFlags[textureIndex] = ASSET_STATE_ENABLE;

    CpuDescriptorHandle descriptor = _descriptors._cpuHandle + incrimentSize * textureIndex;
    device->createShaderResourceView(gpuTexture.getResource(), nullptr, descriptor);
}

void TextureImpl::terminate() {
	_texture->terminate();
}
