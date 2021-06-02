#pragma once
#include <GfxCore/GfxModuleSettings.h>
#include <Core/System.h>

#define ENABLE_MESH_SHADER 1
#define ENABLE_MULTI_INDIRECT_DRAW 1
#define ENABLE_CLASSIC_VERTEX 1

struct Resource;
struct Device;
struct CommandQueue;
struct HardwareFactory;
struct HardwareAdapter;
struct DescriptorHeap;
struct CommandList;
struct PipelineState;
struct RootSignature;
struct CommandSignature;
class QueryHeap;

constexpr u32 DESCRIPTOR_RANGE_OFFSET_APPEND = 0xffffffff;

#define SHADER_COMPONENT_MAPPING_MASK 0x7 
#define SHADER_COMPONENT_MAPPING_SHIFT 3 
#define SHADER_COMPONENT_MAPPING_ALWAYS_SET_BIT_AVOIDING_ZEROMEM_MISTAKES (1<<(SHADER_COMPONENT_MAPPING_SHIFT*4)) 
#define ENCODE_SHADER_4_COMPONENT_MAPPING(Src0,Src1,Src2,Src3) ((((Src0)&SHADER_COMPONENT_MAPPING_MASK)| \
                                                                (((Src1)&SHADER_COMPONENT_MAPPING_MASK)<<SHADER_COMPONENT_MAPPING_SHIFT)| \
                                                                (((Src2)&SHADER_COMPONENT_MAPPING_MASK)<<(SHADER_COMPONENT_MAPPING_SHIFT*2))| \
                                                                (((Src3)&SHADER_COMPONENT_MAPPING_MASK)<<(SHADER_COMPONENT_MAPPING_SHIFT*3))| \
                                                                SHADER_COMPONENT_MAPPING_ALWAYS_SET_BIT_AVOIDING_ZEROMEM_MISTAKES))
#define DECODE_SHADER_4_COMPONENT_MAPPING(ComponentToExtract,Mapping) ((SHADER_COMPONENT_MAPPING)(Mapping >> (SHADER_COMPONENT_MAPPING_SHIFT*ComponentToExtract) & SHADER_COMPONENT_MAPPING_MASK))
#define DEFAULT_SHADER_4_COMPONENT_MAPPING ENCODE_SHADER_4_COMPONENT_MAPPING(0,1,2,3) 

enum Format {
	FORMAT_UNKNOWN = 0,
	FORMAT_R32G32B32A32_TYPELESS = 1,
	FORMAT_R32G32B32A32_FLOAT = 2,
	FORMAT_R32G32B32A32_UINT = 3,
	FORMAT_R32G32B32A32_SINT = 4,
	FORMAT_R32G32B32_TYPELESS = 5,
	FORMAT_R32G32B32_FLOAT = 6,
	FORMAT_R32G32B32_UINT = 7,
	FORMAT_R32G32B32_SINT = 8,
	FORMAT_R16G16B16A16_TYPELESS = 9,
	FORMAT_R16G16B16A16_FLOAT = 10,
	FORMAT_R16G16B16A16_UNORM = 11,
	FORMAT_R16G16B16A16_UINT = 12,
	FORMAT_R16G16B16A16_SNORM = 13,
	FORMAT_R16G16B16A16_SINT = 14,
	FORMAT_R32G32_TYPELESS = 15,
	FORMAT_R32G32_FLOAT = 16,
	FORMAT_R32G32_UINT = 17,
	FORMAT_R32G32_SINT = 18,
	FORMAT_R32G8X24_TYPELESS = 19,
	FORMAT_D32_FLOAT_S8X24_UINT = 20,
	FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	FORMAT_R10G10B10A2_TYPELESS = 23,
	FORMAT_R10G10B10A2_UNORM = 24,
	FORMAT_R10G10B10A2_UINT = 25,
	FORMAT_R11G11B10_FLOAT = 26,
	FORMAT_R8G8B8A8_TYPELESS = 27,
	FORMAT_R8G8B8A8_UNORM = 28,
	FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	FORMAT_R8G8B8A8_UINT = 30,
	FORMAT_R8G8B8A8_SNORM = 31,
	FORMAT_R8G8B8A8_SINT = 32,
	FORMAT_R16G16_TYPELESS = 33,
	FORMAT_R16G16_FLOAT = 34,
	FORMAT_R16G16_UNORM = 35,
	FORMAT_R16G16_UINT = 36,
	FORMAT_R16G16_SNORM = 37,
	FORMAT_R16G16_SINT = 38,
	FORMAT_R32_TYPELESS = 39,
	FORMAT_D32_FLOAT = 40,
	FORMAT_R32_FLOAT = 41,
	FORMAT_R32_UINT = 42,
	FORMAT_R32_SINT = 43,
	FORMAT_R24G8_TYPELESS = 44,
	FORMAT_D24_UNORM_S8_UINT = 45,
	FORMAT_R24_UNORM_X8_TYPELESS = 46,
	FORMAT_X24_TYPELESS_G8_UINT = 47,
	FORMAT_R8G8_TYPELESS = 48,
	FORMAT_R8G8_UNORM = 49,
	FORMAT_R8G8_UINT = 50,
	FORMAT_R8G8_SNORM = 51,
	FORMAT_R8G8_SINT = 52,
	FORMAT_R16_TYPELESS = 53,
	FORMAT_R16_FLOAT = 54,
	FORMAT_D16_UNORM = 55,
	FORMAT_R16_UNORM = 56,
	FORMAT_R16_UINT = 57,
	FORMAT_R16_SNORM = 58,
	FORMAT_R16_SINT = 59,
	FORMAT_R8_TYPELESS = 60,
	FORMAT_R8_UNORM = 61,
	FORMAT_R8_UINT = 62,
	FORMAT_R8_SNORM = 63,
	FORMAT_R8_SINT = 64,
	FORMAT_A8_UNORM = 65,
	FORMAT_R1_UNORM = 66,
	FORMAT_R9G9B9E5_SHAREDEXP = 67,
	FORMAT_R8G8_B8G8_UNORM = 68,
	FORMAT_G8R8_G8B8_UNORM = 69,
	FORMAT_BC1_TYPELESS = 70,
	FORMAT_BC1_UNORM = 71,
	FORMAT_BC1_UNORM_SRGB = 72,
	FORMAT_BC2_TYPELESS = 73,
	FORMAT_BC2_UNORM = 74,
	FORMAT_BC2_UNORM_SRGB = 75,
	FORMAT_BC3_TYPELESS = 76,
	FORMAT_BC3_UNORM = 77,
	FORMAT_BC3_UNORM_SRGB = 78,
	FORMAT_BC4_TYPELESS = 79,
	FORMAT_BC4_UNORM = 80,
	FORMAT_BC4_SNORM = 81,
	FORMAT_BC5_TYPELESS = 82,
	FORMAT_BC5_UNORM = 83,
	FORMAT_BC5_SNORM = 84,
	FORMAT_B5G6R5_UNORM = 85,
	FORMAT_B5G5R5A1_UNORM = 86,
	FORMAT_B8G8R8A8_UNORM = 87,
	FORMAT_B8G8R8X8_UNORM = 88,
	FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	FORMAT_B8G8R8A8_TYPELESS = 90,
	FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	FORMAT_B8G8R8X8_TYPELESS = 92,
	FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	FORMAT_BC6H_TYPELESS = 94,
	FORMAT_BC6H_UF16 = 95,
	FORMAT_BC6H_SF16 = 96,
	FORMAT_BC7_TYPELESS = 97,
	FORMAT_BC7_UNORM = 98,
	FORMAT_BC7_UNORM_SRGB = 99,
	FORMAT_AYUV = 100,
	FORMAT_Y410 = 101,
	FORMAT_Y416 = 102,
	FORMAT_NV12 = 103,
	FORMAT_P010 = 104,
	FORMAT_P016 = 105,
	FORMAT_420_OPAQUE = 106,
	FORMAT_YUY2 = 107,
	FORMAT_Y210 = 108,
	FORMAT_Y216 = 109,
	FORMAT_NV11 = 110,
	FORMAT_AI44 = 111,
	FORMAT_IA44 = 112,
	FORMAT_P8 = 113,
	FORMAT_A8P8 = 114,
	FORMAT_B4G4R4A4_UNORM = 115,

	FORMAT_P208 = 130,
	FORMAT_V208 = 131,
	FORMAT_V408 = 132,


	FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE = 189,
	FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,


	FORMAT_FORCE_UINT = 0xffffffff
};

enum ComparisonFunc {
	COMPARISON_FUNC_NEVER = 1,
	COMPARISON_FUNC_LESS = 2,
	COMPARISON_FUNC_EQUAL = 3,
	COMPARISON_FUNC_LESS_EQUAL = 4,
	COMPARISON_FUNC_GREATER = 5,
	COMPARISON_FUNC_NOT_EQUAL = 6,
	COMPARISON_FUNC_GREATER_EQUAL = 7,
	COMPARISON_FUNC_ALWAYS = 8
};

enum DepthWriteMask {
	DEPTH_WRITE_MASK_ZERO = 0,
	DEPTH_WRITE_MASK_ALL = 1
};

enum PrimitiveTopology {
	PRIMITIVE_TOPOLOGY_UNDEFINED = 0,
	PRIMITIVE_TOPOLOGY_POINTLIST = 1,
	PRIMITIVE_TOPOLOGY_LINELIST = 2,
	PRIMITIVE_TOPOLOGY_LINESTRIP = 3,
	PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4,
	PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5,
	PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10,
	PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ = 11,
	PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ = 12,
	PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ = 13,
};

enum CommandListType {
	COMMAND_LIST_TYPE_DIRECT = 0,
	COMMAND_LIST_TYPE_BUNDLE = 1,
	COMMAND_LIST_TYPE_COMPUTE = 2,
	COMMAND_LIST_TYPE_COPY = 3,
	COMMAND_LIST_TYPE_VIDEO_DECODE = 4,
	COMMAND_LIST_TYPE_VIDEO_PROCESS = 5,
	COMMAND_LIST_TYPE_VIDEO_ENCODE = 6
};

enum DescriptorHeapFlag {
	DESCRIPTOR_HEAP_FLAG_NONE = 0,
	DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE = 0x1,
};

enum DescriptorHeapType {
	DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV = 0,
	DESCRIPTOR_HEAP_TYPE_SAMPLER = (DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV + 1),
	DESCRIPTOR_HEAP_TYPE_RTV = (DESCRIPTOR_HEAP_TYPE_SAMPLER + 1),
	DESCRIPTOR_HEAP_TYPE_DSV = (DESCRIPTOR_HEAP_TYPE_RTV + 1),
	DESCRIPTOR_HEAP_TYPE_NUM_TYPES = (DESCRIPTOR_HEAP_TYPE_DSV + 1)
};

enum ResourceStates {
	RESOURCE_STATE_COMMON = 0,
	RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
	RESOURCE_STATE_INDEX_BUFFER = 0x2,
	RESOURCE_STATE_RENDER_TARGET = 0x4,
	RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
	RESOURCE_STATE_DEPTH_WRITE = 0x10,
	RESOURCE_STATE_DEPTH_READ = 0x20,
	RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
	RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
	RESOURCE_STATE_STREAM_OUT = 0x100,
	RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
	RESOURCE_STATE_COPY_DEST = 0x400,
	RESOURCE_STATE_COPY_SOURCE = 0x800,
	RESOURCE_STATE_RESOLVE_DEST = 0x1000,
	RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
	RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
	RESOURCE_STATE_SHADING_RATE_SOURCE = 0x1000000,
	RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
	RESOURCE_STATE_ALL_SHADER_RESOURCE = (0x40 | 0x80),
	RESOURCE_STATE_PRESENT = 0,
	RESOURCE_STATE_PREDICATION = 0x200,
	RESOURCE_STATE_VIDEO_DECODE_READ = 0x10000,
	RESOURCE_STATE_VIDEO_DECODE_WRITE = 0x20000,
	RESOURCE_STATE_VIDEO_PROCESS_READ = 0x40000,
	RESOURCE_STATE_VIDEO_PROCESS_WRITE = 0x80000,
	RESOURCE_STATE_VIDEO_ENCODE_READ = 0x200000,
	RESOURCE_STATE_VIDEO_ENCODE_WRITE = 0x800000
};

enum DescriptorRangeType {
	DESCRIPTOR_RANGE_TYPE_SRV = 0,
	DESCRIPTOR_RANGE_TYPE_UAV = (DESCRIPTOR_RANGE_TYPE_SRV + 1),
	DESCRIPTOR_RANGE_TYPE_CBV = (DESCRIPTOR_RANGE_TYPE_UAV + 1),
	DESCRIPTOR_RANGE_TYPE_SAMPLER = (DESCRIPTOR_RANGE_TYPE_CBV + 1)
};

enum DescriptorRangeFlags {
	DESCRIPTOR_RANGE_FLAG_NONE = 0,
	DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE = 0x1,
	DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE = 0x2,
	DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE = 0x4,
	DESCRIPTOR_RANGE_FLAG_DATA_STATIC = 0x8,
	DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS = 0x10000
};

enum RootDescriptorFlags {
	ROOT_DESCRIPTOR_FLAG_NONE = 0,
	ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE = 0x2,
	ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE = 0x4,
	ROOT_DESCRIPTOR_FLAG_DATA_STATIC = 0x8
};

enum RootSignatureFlags {
	ROOT_SIGNATURE_FLAG_NONE = 0,
	ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT = 0x1,
	ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS = 0x2,
	ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS = 0x4,
	ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS = 0x8,
	ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS = 0x10,
	ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS = 0x20,
	ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT = 0x40,
	ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE = 0x80,
	ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS = 0x100,
	ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS = 0x200
};

enum HeapType {
	HEAP_TYPE_DEFAULT = 1,
	HEAP_TYPE_UPLOAD = 2,
	HEAP_TYPE_READBACK = 3,
	HEAP_TYPE_CUSTOM = 4
};

enum HeapFlags {
	HEAP_FLAG_NONE = 0,
	HEAP_FLAG_SHARED = 0x1,
	HEAP_FLAG_DENY_BUFFERS = 0x4,
	HEAP_FLAG_ALLOW_DISPLAY = 0x8,
	HEAP_FLAG_SHARED_CROSS_ADAPTER = 0x20,
	HEAP_FLAG_DENY_RT_DS_TEXTURES = 0x40,
	HEAP_FLAG_DENY_NON_RT_DS_TEXTURES = 0x80,
	HEAP_FLAG_HARDWARE_PROTECTED = 0x100,
	HEAP_FLAG_ALLOW_WRITE_WATCH = 0x200,
	HEAP_FLAG_ALLOW_SHADER_ATOMICS = 0x400,
	HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES = 0,
	HEAP_FLAG_ALLOW_ONLY_BUFFERS = 0xc0,
	HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES = 0x44,
	HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES = 0x84
};

enum ResourceDimension {
	RESOURCE_DIMENSION_UNKNOWN = 0,
	RESOURCE_DIMENSION_BUFFER = 1,
	RESOURCE_DIMENSION_TEXTURE1D = 2,
	RESOURCE_DIMENSION_TEXTURE2D = 3,
	RESOURCE_DIMENSION_TEXTURE3D = 4
};

enum TextureLayout {
	TEXTURE_LAYOUT_UNKNOWN = 0,
	TEXTURE_LAYOUT_ROW_MAJOR = 1,
	TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE = 2,
	TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE = 3
};

enum ResourceFlags {
	RESOURCE_FLAG_NONE = 0,
	RESOURCE_FLAG_ALLOW_RENDER_TARGET = 0x1,
	RESOURCE_FLAG_ALLOW_DEPTH_STENCIL = 0x2,
	RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS = 0x4,
	RESOURCE_FLAG_DENY_SHADER_RESOURCE = 0x8,
	RESOURCE_FLAG_ALLOW_CROSS_ADAPTER = 0x10,
	RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS = 0x20,
	RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY = 0x40
};

enum BufferSrvFlags {
	BUFFER_SRV_FLAG_NONE = 0,
	BUFFER_SRV_FLAG_RAW = 0x1
};

enum IndirectArgumentType {
	INDIRECT_ARGUMENT_TYPE_DRAW = 0,
	INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED = (INDIRECT_ARGUMENT_TYPE_DRAW + 1),
	INDIRECT_ARGUMENT_TYPE_DISPATCH = (INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED + 1),
	INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW = (INDIRECT_ARGUMENT_TYPE_DISPATCH + 1),
	INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW = (INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW + 1),
	INDIRECT_ARGUMENT_TYPE_CONSTANT = (INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW + 1),
	INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW = (INDIRECT_ARGUMENT_TYPE_CONSTANT + 1),
	INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW = (INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW + 1),
	INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW = (INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW + 1),
	INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS = (INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW + 1),
	INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH = (INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS + 1)
};

enum ClearFlags {
	CLEAR_FLAG_DEPTH = 0x1,
	CLEAR_FLAG_STENCIL = 0x2
};

enum TextureCopyType {
	TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX = 0,
	TEXTURE_COPY_TYPE_PLACED_FOOTPRINT = 1
};

enum Blend {
	BLEND_ZERO = 1,
	BLEND_ONE = 2,
	BLEND_SRC_COLOR = 3,
	BLEND_INV_SRC_COLOR = 4,
	BLEND_SRC_ALPHA = 5,
	BLEND_INV_SRC_ALPHA = 6,
	BLEND_DEST_ALPHA = 7,
	BLEND_INV_DEST_ALPHA = 8,
	BLEND_DEST_COLOR = 9,
	BLEND_INV_DEST_COLOR = 10,
	BLEND_SRC_ALPHA_SAT = 11,
	BLEND_BLEND_FACTOR = 14,
	BLEND_INV_BLEND_FACTOR = 15,
	BLEND_SRC1_COLOR = 16,
	BLEND_INV_SRC1_COLOR = 17,
	BLEND_SRC1_ALPHA = 18,
	BLEND_INV_SRC1_ALPHA = 19
};

enum BlendOp {
	BLEND_OP_ADD = 1,
	BLEND_OP_SUBTRACT = 2,
	BLEND_OP_REV_SUBTRACT = 3,
	BLEND_OP_MIN = 4,
	BLEND_OP_MAX = 5
};

enum LogicOp {
	LOGIC_OP_CLEAR = 0,
	LOGIC_OP_SET = (LOGIC_OP_CLEAR + 1),
	LOGIC_OP_COPY = (LOGIC_OP_SET + 1),
	LOGIC_OP_COPY_INVERTED = (LOGIC_OP_COPY + 1),
	LOGIC_OP_NOOP = (LOGIC_OP_COPY_INVERTED + 1),
	LOGIC_OP_INVERT = (LOGIC_OP_NOOP + 1),
	LOGIC_OP_AND = (LOGIC_OP_INVERT + 1),
	LOGIC_OP_NAND = (LOGIC_OP_AND + 1),
	LOGIC_OP_OR = (LOGIC_OP_NAND + 1),
	LOGIC_OP_NOR = (LOGIC_OP_OR + 1),
	LOGIC_OP_XOR = (LOGIC_OP_NOR + 1),
	LOGIC_OP_EQUIV = (LOGIC_OP_XOR + 1),
	LOGIC_OP_AND_REVERSE = (LOGIC_OP_EQUIV + 1),
	LOGIC_OP_AND_INVERTED = (LOGIC_OP_AND_REVERSE + 1),
	LOGIC_OP_OR_REVERSE = (LOGIC_OP_AND_INVERTED + 1),
	LOGIC_OP_OR_INVERTED = (LOGIC_OP_OR_REVERSE + 1)
};

enum ColorWriteEnable{
	COLOR_WRITE_ENABLE_RED = 1,
	COLOR_WRITE_ENABLE_GREEN = 2,
	COLOR_WRITE_ENABLE_BLUE = 4,
	COLOR_WRITE_ENABLE_ALPHA = 8,
	COLOR_WRITE_ENABLE_ALL = (((COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_GREEN) | COLOR_WRITE_ENABLE_BLUE) | COLOR_WRITE_ENABLE_ALPHA)
};

struct QueryVideoMemoryInfo {
	u64 _budget = 0;
	u64 _currentUsage = 0;
	u64 _availableForReservation = 0;
	u64 _currentReservation = 0;
};

struct RenderTargetBlendDesc {
	bool _blendEnable = false;
	bool _logicOpEnable = false;
	Blend _srcBlend = BLEND_ONE;
	Blend _destBlend = BLEND_ZERO;
	BlendOp _blendOp = BLEND_OP_ADD;
	Blend _srcBlendAlpha = BLEND_ONE;
	Blend _destBlendAlpha = BLEND_ZERO;
	BlendOp _blendOpAlpha = BLEND_OP_ADD;
	LogicOp _logicOp = LOGIC_OP_NOOP;
	u8 _renderTargetWriteMask = COLOR_WRITE_ENABLE_ALL;
};

struct BlendDesc {
	bool _alphaToCoverageEnable = false;
	bool _independentBlendEnable = false;
	RenderTargetBlendDesc _renderTarget[8] = {};
};

struct SubresourceFootprint {
	Format _format;
	u32 _width = 0;
	u32 _height = 0;
	u32 _depth = 0;
	u32 _rowPitch = 0;
};

struct PlacedSubresourceFootprint {
	u64 _offset = 0;
	SubresourceFootprint _footprint;
};

struct TextureCopyLocation {
	Resource* _resource;
	TextureCopyType _type;
	union {
		PlacedSubresourceFootprint _placedFootprint;
		u32 _subresourceIndex;
	};
};

struct SubResourceData {
	const void* _data = nullptr;
	s64 _rowPitch = 0;
	s64 _slicePitch = 0;
};

struct MemcpyDest {
	void* _data = nullptr;
	u64 _rowPitch = 0;
	u64 _slicePitch = 0;
};

struct BufferSrv {
	u64 _firstElement = 0;
	u32 _numElements = 0;
	u32 _structureByteStride = 0;
	BufferSrvFlags _flags = BUFFER_SRV_FLAG_NONE;
};

struct Tex1dSrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	f32 _resourceMinLODClamp = 0;
};

struct Tex1dArraySrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	u32 _firstArraySlice = 0;
	u32 _arraySize = 0;
	f32 _resourceMinLODClamp = 0.0f;
};

struct Tex2dSrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	u32 _planeSlice = 0;
	f32 _resourceMinLODClamp = 0.0f;
};

struct Tex2dArraySrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	u32 _firstArraySlice = 0;
	u32 _arraySize = 0;
	u32 _planeSlice = 0;
	f32 _resourceMinLODClamp = 0;
};

struct Tex3dSrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	f32 _resourceMinLODClamp = 0;
};

struct TexCubeSrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	f32 _resourceMinLODClamp = 0;
};

struct TexCubeArraySrv {
	u32 _mostDetailedMip = 0;
	u32 _mipLevels = 0;
	u32 _first2DArrayFace = 0;
	u32 _numCubes = 0;
	f32 _resourceMinLODClamp = 0;
};

struct Tex2dmsSrv {
	u32 _unusedFieldNothingToDefine = 0;
};

struct Tex2dmsArraySrv {
	u32 _firstArraySlice = 0;
	u32 _arraySize = 0;
};

struct RayTracingAccelerationStructureSrv{
	u64 _location = 0;
};

enum SrvDimension {
	SRV_DIMENSION_UNKNOWN = 0,
	SRV_DIMENSION_BUFFER = 1,
	SRV_DIMENSION_TEXTURE1D = 2,
	SRV_DIMENSION_TEXTURE1DARRAY = 3,
	SRV_DIMENSION_TEXTURE2D = 4,
	SRV_DIMENSION_TEXTURE2DARRAY = 5,
	SRV_DIMENSION_TEXTURE2DMS = 6,
	SRV_DIMENSION_TEXTURE2DMSARRAY = 7,
	SRV_DIMENSION_TEXTURE3D = 8,
	SRV_DIMENSION_TEXTURECUBE = 9,
	SRV_DIMENSION_TEXTURECUBEARRAY = 10,
	SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE = 11
};

enum InputClassFication {
	INPUT_CLASSIFICATION_PER_VERTEX_DATA = 0,
	INPUT_CLASSIFICATION_PER_INSTANCE_DATA = 1
};

struct InputElementDesc {
	const char* _semanticName = nullptr;
	u32 _semanticIndex = 0;
	Format _format = FORMAT_UNKNOWN;
	u32 _inputSlot = 0;
	u32 _alignedByteOffset = 0;
	InputClassFication _inputSlotClass = INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	u32 _instanceDataStepRate = 0;
};

struct DescriptorRange {
	DescriptorRange(DescriptorRangeType rangeType,
		u32 numDescriptors,
		u32 baseShaderRegister,
		DescriptorRangeFlags flags = DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE)
		:_rangeType(rangeType), _numDescriptors(numDescriptors), _baseShaderRegister(baseShaderRegister),_flags(flags){}

	void initialize(DescriptorRangeType rangeType,
		u32 numDescriptors,
		u32 baseShaderRegister,
		DescriptorRangeFlags flags = DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE) {
		_rangeType = rangeType;
		_numDescriptors = numDescriptors;
		_baseShaderRegister = baseShaderRegister;
		_flags = flags;
	}

	DescriptorRangeType _rangeType;
	u32 _numDescriptors = 0;
	u32 _baseShaderRegister = 0;
	u32 _registerSpace = 0;
	DescriptorRangeFlags _flags;
	u32 _offsetInDescriptorsFromTableStart = DESCRIPTOR_RANGE_OFFSET_APPEND;
};

struct DepthStencilValue {
	f32 _depth = 0.0f;
	u8 _stencil = 0;
};

struct ClearValue {
	Format _format;
	union {
		f32 _color[4] = {};
		DepthStencilValue _depthStencil;
	};
};

struct SampleDesc {
	u32 _count = 0;
	u32 _quality = 0;
};

struct ResourceDesc {
	ResourceDimension _dimension;
	u64 _alignment = 0;
	u64 _width = 0;
	u32 _height = 0;
	u16 _depthOrArraySize = 0;
	u16 _mipLevels = 0;
	Format _format;
	SampleDesc _sampleDesc;
	TextureLayout _layout;
	ResourceFlags _flags;
};

enum ShaderVisiblity {
	SHADER_VISIBILITY_ALL = 0,
	SHADER_VISIBILITY_VERTEX = 1,
	SHADER_VISIBILITY_HULL = 2,
	SHADER_VISIBILITY_DOMAIN = 3,
	SHADER_VISIBILITY_GEOMETRY = 4,
	SHADER_VISIBILITY_PIXEL = 5,
	SHADER_VISIBILITY_AMPLIFICATION = 6,
	SHADER_VISIBILITY_MESH = 7
};

enum RootParameterType {
	ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE = 0,
	ROOT_PARAMETER_TYPE_32BIT_CONSTANTS = (ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE + 1),
	ROOT_PARAMETER_TYPE_CBV = (ROOT_PARAMETER_TYPE_32BIT_CONSTANTS + 1),
	ROOT_PARAMETER_TYPE_SRV = (ROOT_PARAMETER_TYPE_CBV + 1),
	ROOT_PARAMETER_TYPE_UAV = (ROOT_PARAMETER_TYPE_SRV + 1)
};

enum PipelineStateFlags {
	PIPELINE_STATE_FLAG_NONE = 0,
	PIPELINE_STATE_FLAG_TOOL_DEBUG = 0x1
};

struct RootDescriptorTable {
	u32 _numDescriptorRanges = 0;
	const DescriptorRange* _descriptorRanges = nullptr;
};

struct RootConstants {
	u32 _shaderRegister = 0;
	u32 _registerSpace = 0;
	u32 _num32BitValues = 0;
};

struct RootDescriptor {
	u32 _shaderRegister = 0;
	u32 _registerSpace = 0;
	RootDescriptorFlags _flags;
};

struct RootParameter {
	void initializeDescriptorTable(
		u32 numDescriptorRange,
		const DescriptorRange* descriptorRanges,
		ShaderVisiblity shaderVisibility) {
		_parameterType = ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		_descriptorTable._numDescriptorRanges = numDescriptorRange;
		_descriptorTable._descriptorRanges = descriptorRanges;
		_shaderVisibility = shaderVisibility;
	}

	void initializeDescriptorCbv(
		u32 shaderRegister,
		ShaderVisiblity shaderVisibility,
		RootDescriptorFlags flags = ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE) {
		_parameterType = ROOT_PARAMETER_TYPE_CBV;
		_descriptor._shaderRegister = shaderRegister;
		_descriptor._flags = flags;
		_shaderVisibility = shaderVisibility;
	}

	void initializeDescriptorSrv(
		u32 shaderRegister,
		ShaderVisiblity shaderVisibility,
		RootDescriptorFlags flags = ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE) {
		_parameterType = ROOT_PARAMETER_TYPE_SRV;
		_descriptor._shaderRegister = shaderRegister;
		_descriptor._flags = flags;
		_shaderVisibility = shaderVisibility;
	}

	void initializeConstant(
		u32 shaderRegister,
		u32 num32bitValues,
		ShaderVisiblity shaderVisibility,
		RootDescriptorFlags flags = ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE) {
		_parameterType = ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		_shaderVisibility = shaderVisibility;
		_constants._num32BitValues = num32bitValues;
		_constants._shaderRegister = shaderRegister;
	}

	RootParameterType _parameterType;
	RootDescriptorTable _descriptorTable;
	RootConstants _constants;
	RootDescriptor _descriptor;
	ShaderVisiblity _shaderVisibility;
};

struct ShaderByteCode {
	const void* _shaderBytecode = nullptr;
	u64 _bytecodeLength = 0;
};

enum PrimitiveTopologyType {
	PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED = 0,
	PRIMITIVE_TOPOLOGY_TYPE_POINT = 1,
	PRIMITIVE_TOPOLOGY_TYPE_LINE = 2,
	PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE = 3,
	PRIMITIVE_TOPOLOGY_TYPE_PATCH = 4
};

enum UavDimension {
	UAV_DIMENSION_UNKNOWN = 0,
	UAV_DIMENSION_BUFFER = 1,
	UAV_DIMENSION_TEXTURE1D = 2,
	UAV_DIMENSION_TEXTURE1DARRAY = 3,
	UAV_DIMENSION_TEXTURE2D = 4,
	UAV_DIMENSION_TEXTURE2DARRAY = 5,
	UAV_DIMENSION_TEXTURE3D = 8
};

enum BufferUavFlags {
	BUFFER_UAV_FLAG_NONE = 0,
	BUFFER_UAV_FLAG_RAW = 0x1
};

enum QueryHeapType {
	QUERY_HEAP_TYPE_OCCLUSION = 0,
	QUERY_HEAP_TYPE_TIMESTAMP = 1,
	QUERY_HEAP_TYPE_PIPELINE_STATISTICS = 2,
	QUERY_HEAP_TYPE_SO_STATISTICS = 3,
	QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS = 4,
	QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP = 5
};

enum QueryType {
	QUERY_TYPE_OCCLUSION = 0,
	QUERY_TYPE_BINARY_OCCLUSION = 1,
	QUERY_TYPE_TIMESTAMP = 2,
	QUERY_TYPE_PIPELINE_STATISTICS = 3,
	QUERY_TYPE_SO_STATISTICS_STREAM0 = 4,
	QUERY_TYPE_SO_STATISTICS_STREAM1 = 5,
	QUERY_TYPE_SO_STATISTICS_STREAM2 = 6,
	QUERY_TYPE_SO_STATISTICS_STREAM3 = 7,
	QUERY_TYPE_VIDEO_DECODE_STATISTICS = 8
};

enum FillMode {
	FILL_MODE_WIREFRAME = 2,
	FILL_MODE_SOLID = 3
};

struct BufferUav {
	u64 _firstElement = 0;
	u32 _numElements = 0;
	u32 _structureByteStride = 0;
	u64 _counterOffsetInBytes = 0;
	BufferUavFlags _flags;
};

struct Tex1dUav {
	u32 _mipSlice = 0;
};

struct Tex1dArrayUav {
	u32 _mipSlice = 0;
	u32 _firstArraySlice = 0;
	u32 _arraySize = 0;
};

struct Tex2dUav {
	u32 _mipSlice = 0;
	u32 _planeSlice = 0;
};

struct Tex2dArrayUav {
	u32 _mipSlice = 0;
	u32 _firstArraySlice = 0;
	u32 _arraySize = 0;
	u32 _planeSlice = 0;
};

struct Tex3dUav {
	u32 _mipSlice = 0;
	u32 _firstWSlice = 0;
	u32 _wSize = 0;
};

struct UnorderedAccessViewDesc {
	Format _format;
	UavDimension _viewDimension;
	union {
		BufferUav _buffer;
		Tex1dUav _texture1D;
		Tex1dArrayUav _texture1DArray;
		Tex2dUav _texture2D;
		Tex2dArrayUav _texture2DArray;
		Tex3dUav _texture3D;
	};
};

struct GraphicsPipelineStateDesc {
	Device* _device = nullptr;
	RootSignature* _rootSignature = nullptr;
	ShaderByteCode _vs;
	ShaderByteCode _ps;
	ShaderByteCode _ds;
	ShaderByteCode _hs;
	ShaderByteCode _gs;

	const InputElementDesc* _inputElements;
	u32 _inputElementCount = 0;
	u32 _numRenderTarget = 0;
	Format _rtvFormats[8] = {};
	Format _dsvFormat;
	ComparisonFunc _depthComparisonFunc = COMPARISON_FUNC_LESS_EQUAL;
	DepthWriteMask _depthWriteMask = DEPTH_WRITE_MASK_ALL;
	SampleDesc _sampleDesc;
	PrimitiveTopologyType _topologyType;
};

#if ENABLE_MESH_SHADER
struct MeshPipelineStateDesc {
	Device* _device = nullptr;
	RootSignature* _rootSignature = nullptr;
	ShaderByteCode _ms;
	ShaderByteCode _as;
	ShaderByteCode _ps;

	u32 _numRenderTarget = 0;
	Format _rtvFormats[8] = {};
	Format _dsvFormat;
	ComparisonFunc _depthComparisonFunc;
	SampleDesc _sampleDesc;
	PrimitiveTopologyType _topologyType;
	BlendDesc _blendDesc;
	FillMode _fillMode = FILL_MODE_SOLID;
};
#endif

struct CachedPipelineState {
	const void* cachedBlob = nullptr;
	u64 _cachedBlobSizeInBytes = 0;
};

struct ComputePipelineStateDesc {
	Device* _device = nullptr;
	RootSignature* _rootSignature = nullptr;
	ShaderByteCode _cs;
	u32 _nodeMask = 0;
	CachedPipelineState _cachedPSO;
	PipelineStateFlags _flags;
};

struct StaticSamplerDesc {
};

struct RootSignatureDesc {
	Device* _device = nullptr;
	u32 _numParameters;
	const RootParameter* _parameters;
	u32 _numStaticSamplers;
	const StaticSamplerDesc* _staticSamplers = nullptr;
	RootSignatureFlags _flags;
};

struct HardwareFactoryDesc {
	enum FACTROY_FLAG {
		FACTROY_FLAG_NONE = 0,
		FACTROY_FLGA_DEVICE_DEBUG = 1 << 0,
	};

	u8 _flags;
};

struct HardwareAdapterDesc {
	HardwareFactory* _factory = nullptr;
};

struct DeviceDesc {
	HardwareAdapter* _adapter = nullptr;
	const char* _debugName = nullptr;
};

struct CommandQueueDesc {
	Device* _device = nullptr;
	CommandListType _type = COMMAND_LIST_TYPE_DIRECT;
	const char* _debugName = nullptr;
};

struct DescriptorHeapDesc {
	Device* _device = nullptr;
	u32 _numDescriptors;
	DescriptorHeapType _type;
	DescriptorHeapFlag _flags;
};

struct CommandListDesc {
	Device* _device = nullptr;
	CommandListType _type = COMMAND_LIST_TYPE_DIRECT;
	const char* _debugName = nullptr;
};

struct SwapChainDesc {
	HardwareFactory* _factory = nullptr;
	CommandQueue* _commandQueue = nullptr;
	u32 _bufferingCount = 0;
	u32 _width = 0;
	u32 _height = 0;
	s32* _hWnd = nullptr;
	Format _format;
};

struct ViewPort {
	f32 _topLeftX = 0.0f;
	f32 _topLeftY = 0.0f;
	f32 _width = 0.0f;
	f32 _height = 0.0f;
	f32 _minDepth = 0.0f;
	f32 _maxDepth = 1.0f;
};

struct Rect {
	l32 _left = 0;
	l32 _top = 0;
	l32 _right = 0;
	l32 _bottom = 0;
};

struct Box {
	u32 _left = 0;
	u32 _top = 0;
	u32 _front = 0;
	u32 _right = 0;
	u32 _bottom = 0;
	u32 _back = 0;
};

struct VertexBufferView {
	u64 _bufferLocation = 0;
	u32 _sizeInBytes = 0;
	u32 _strideInBytes = 0;
};

struct IndexBufferView {
	u64 _bufferLocation = 0;
	u32 _sizeInBytes = 0;
	Format _format;
};

struct MemoryRange {
	MemoryRange(u64 begin, u64 end):_begin(begin),_end(end){}
	u64 _begin = 0;
	u64 _end = 0;
};

struct ConstantBufferViewDesc {
	u64 _bufferLocation;
	u32 _sizeInBytes;
};

struct ShaderResourceViewDesc {
	Format _format = FORMAT_UNKNOWN;
	SrvDimension _viewDimension = SRV_DIMENSION_UNKNOWN;
	u32 _shader4ComponentMapping = DEFAULT_SHADER_4_COMPONENT_MAPPING;
	union {
		BufferSrv _buffer;
		Tex1dSrv _texture1D;
		Tex1dArraySrv _texture1DArray;
		Tex2dSrv _texture2D;
		Tex2dArraySrv _texture2DArray;
		Tex2dmsSrv _texture2DMS;
		Tex2dmsArraySrv _texture2DMSArray;
		Tex3dSrv _texture3D;
		TexCubeSrv _textureCube;
		TexCubeArraySrv _textureCubeArray;
		RayTracingAccelerationStructureSrv _raytracingAccelerationStructure;
	};
};

struct ResourceTransitionBarrier {
	Resource* _resource = nullptr;
	u32 _subresource = 0;
	ResourceStates _stateBefore;
	ResourceStates _stateAfter;
};

struct CpuDescriptorHandle {
	CpuDescriptorHandle operator + (u64 offset) const {
		CpuDescriptorHandle result = *this;
		result._ptr += offset;
		return result;
	}

	CpuDescriptorHandle& operator+=(u64 offset){
		_ptr = _ptr + offset;
		return *this;
	}

	u64 _ptr = 0;
};

struct GpuDescriptorHandle {
	GpuDescriptorHandle operator + (u64 offset) const {
		GpuDescriptorHandle result = *this;
		result._ptr += offset;
		return result;
	}

	GpuDescriptorHandle& operator+=(u64 offset) {
		_ptr = _ptr + offset;
		return *this;
	}

	u64 _ptr = 0;
};

struct DescriptorHandle {
	DescriptorHandle operator + (u64 offset) const {
		DescriptorHandle handle = *this;
		handle += offset;
		return handle;
	}

	DescriptorHandle& operator += (u64 offset) {
		_cpuHandle = _cpuHandle + offset;
		_gpuHandle = _gpuHandle + offset;
		return *this;
	}

	u32 _numDescriptor = 0;
	CpuDescriptorHandle _cpuHandle;
	GpuDescriptorHandle _gpuHandle;
};

struct IndirectArgumentDesc {
	IndirectArgumentType _type;
	union {
		struct {
			u32 _slot;
		} VertexBuffer;
		struct {
			u32 _rootParameterIndex;
			u32 _destOffsetIn32BitValues;
			u32 _num32BitValuesToSet;
		} Constant;
		struct {
			u32 _rootParameterIndex;
		} ConstantBufferView;
		struct {
			u32 _rootParameterIndex;
		} ShaderResourceView;
		struct {
			u32 _rootParameterIndex;
		} UnorderedAccessView;
	};
};

struct CommandSignatureDesc {
	Device* _device = nullptr;
	RootSignature* _rootSignature = nullptr;
	u32 _byteStride = 0;
	u32 _numArgumentDescs = 0;
	const IndirectArgumentDesc* _argumentDescs = nullptr;
	u32 _nodeMask = 0;
};

struct QueryHeapDesc {
	Device* _device = nullptr;
	QueryHeapType _type;
	u32 _count = 0;
	u32 _nodeMask = 0;
};

struct DrawIndexedArguments {
	u32 _indexCountPerInstance;
	u32 _instanceCount;
	u32 _startIndexLocation;
	s32 _baseVertexLocation;
	u32 _startInstanceLocation;
};

struct DrawArguments {
	u32 _vertexCountPerInstance;
	u32 _instanceCount;
	u32 _startVertexLocation;
	u32 _startInstanceLocation;
};

struct LTN_GFX_API ShaderBlob {
	virtual void initialize(const char* filePath) = 0;
	virtual void terminate() = 0;
	virtual ShaderByteCode getShaderByteCode() const = 0;
};

struct LTN_GFX_API HardwareFactory {
	virtual void initialize(const HardwareFactoryDesc& desc) = 0;
	virtual void terminate() = 0;
};
struct LTN_GFX_API HardwareAdapter {
	virtual void initialize(const HardwareAdapterDesc& desc) = 0;
	virtual void terminate() = 0;
	virtual QueryVideoMemoryInfo queryVideoMemoryInfo() = 0;
};

struct LTN_GFX_API Device {
	virtual void initialize(const DeviceDesc& desc) = 0;
	virtual void terminate() = 0;

	virtual u32 getDescriptorHandleIncrementSize(DescriptorHeapType type) const = 0;
	virtual void createRenderTargetView(Resource* resource, CpuDescriptorHandle destDescriptor) = 0;
	virtual void createDepthStencilView(Resource* resource, CpuDescriptorHandle destDescriptor) = 0;
	virtual void createCommittedResource(HeapType heapType, HeapFlags heapFlags, const ResourceDesc& desc,
		ResourceStates initialResourceState, const ClearValue* optimizedClearValue, Resource* dstResource) = 0;
	virtual void createConstantBufferView(const ConstantBufferViewDesc& desc, CpuDescriptorHandle destDescriptor) = 0;
	virtual void createShaderResourceView(Resource* resource, const ShaderResourceViewDesc* desc, CpuDescriptorHandle destDescriptor) = 0;
	virtual void createUnorderedAccessView(Resource* resource, Resource* counterResource, const UnorderedAccessViewDesc* desc, CpuDescriptorHandle destDescriptor) = 0;
	virtual void getCopyableFootprints(const ResourceDesc* resourceDesc,u32 firstSubresource, u32 numSubresources, u64 baseOffset, PlacedSubresourceFootprint* layouts, u32* numRows, u64* rowSizeInBytes, u64* totalBytes) = 0;
	virtual u8 getFormatPlaneCount(Format format) = 0;
};

struct LTN_GFX_API SwapChain {
	virtual void initialize(const SwapChainDesc& desc) = 0;
	virtual void terminate() = 0;

	virtual u32 getCurrentBackBufferIndex() = 0;
	virtual void present(u32 syncInterval, u32 flags) = 0;
	virtual void getBackBuffer(Resource** resource, u32 index) = 0;
};

struct LTN_GFX_API CommandQueue {
	virtual void initialize(const CommandQueueDesc& desc) = 0;
	virtual void terminate() = 0;

	virtual void getTimestampFrequency(u64* frequency) = 0;
	virtual void waitForIdle() = 0;
	virtual void waitForFence(u64 fenceValue) = 0;
	virtual void executeCommandLists(u32 count, CommandList** commandLists) = 0;
	virtual u64 incrimentFence() = 0;
	virtual u64 getCompleatedValue() const = 0;
};

struct LTN_GFX_API DescriptorHeap {
	virtual void initialize(const DescriptorHeapDesc& desc) = 0;
	virtual void terminate() = 0;

	virtual u32 getIncrimentSize() const = 0;
	virtual CpuDescriptorHandle getCPUDescriptorHandleForHeapStart() const = 0;
	virtual GpuDescriptorHandle getGPUDescriptorHandleForHeapStart() const = 0;
};

struct LTN_GFX_API CommandList {
	virtual void initialize(const CommandListDesc& desc) = 0;
	virtual void terminate() = 0;

	virtual void transitionBarrierSimple(Resource* resource, ResourceStates currentState, ResourceStates nextState) = 0;
	virtual void transitionBarriers(ResourceTransitionBarrier* barriers, u32 count) = 0;
	virtual void copyBufferRegion(Resource* dstBuffer, u64 dstOffset, Resource* srcBuffer, u64 srcOffset, u64 numBytes) = 0;
	virtual void copyResource(Resource* dstResource, Resource* srcResource) = 0;
	virtual void copyTextureRegion(const TextureCopyLocation* dst, u32 dstX, u32 dstY, u32 dstZ, const TextureCopyLocation* src, const Box* srcBox) = 0;
	virtual void setDescriptorHeaps(u32 count, DescriptorHeap** descriptorHeaps) = 0;
	virtual void setViewports(u32 count, const ViewPort* viewPorts) = 0;
	virtual void setScissorRects(u32 count, const Rect* scissorRects) = 0;
	virtual void setRenderTargets(u32 count, DescriptorHandle* rtvHandles, DescriptorHandle* dsvHandle) = 0;
	virtual void clearRenderTargetView(DescriptorHandle rtvHandle, f32 clearColor[4]) = 0;
	virtual void clearDepthStencilView(CpuDescriptorHandle depthStencilView, ClearFlags clearFlags, f32 depth, u8 stencil, u32 numRects, const Rect* rects) = 0;
	virtual void setPipelineState(PipelineState* pipelineState) = 0;
	virtual void setGraphicsRootSignature(RootSignature* rootSignature) = 0;
	virtual void setComputeRootSignature(RootSignature* rootSignature) = 0;
	virtual void setPrimitiveTopology(PrimitiveTopology primitiveTopology) = 0;
	virtual void setGraphicsRootDescriptorTable(u32 rootParameterIndex, GpuDescriptorHandle baseDescriptor) = 0;
	virtual void setComputeRootDescriptorTable(u32 rootParameterIndex, GpuDescriptorHandle baseDescriptor) = 0;
	virtual void setGraphicsRoot32BitConstants(u32 rootParameterIndex, u32 num32BitValuesToSet, const void* srcData, u32 destOffsetIn32BitValues) = 0;
	virtual void drawInstanced(u32 vertexCountPerInstance, u32 instanceCount, u32 startVertexLocation, u32 startInstanceLocation) = 0;
	virtual void drawIndexedInstanced(u32 indexCountPerInstance, u32 instanceCount, u32 startIndexLocation, s32 baseVertexLocation, u32 startInstanceLocation) = 0;
	virtual void dispatch(u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) = 0;
	virtual void executeIndirect(CommandSignature* commandSignature, u32 maxCommandCount, Resource* argumentBuffer, u64 argumentBufferOffset, Resource* countBuffer, u64 countBufferOffset) = 0;
	virtual void clearUnorderedAccessViewUint(GpuDescriptorHandle viewGPUHandleInCurrentHeap, CpuDescriptorHandle viewCPUHandle, Resource* resource, const u32 values[4], u32 numRects, const Rect* rects) = 0;
	virtual void setVertexBuffers(u32 startSlot, u32 numViews, const VertexBufferView* views) = 0;
	virtual void setIndexBuffer(const IndexBufferView* view) = 0;
	virtual void endQuery(QueryHeap* queryHeap, QueryType type, u32 index) = 0;
	virtual void resolveQueryData(QueryHeap* queryHeap, QueryType type, u32 startIndex, u32 numQueries, Resource* destinationBuffer, u64 alignedDestinationBufferOffset) = 0;
#if ENABLE_MESH_SHADER
	virtual void dispatchMesh(u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) = 0;
#endif
	//virtual void clearDepthStencilView(CpuDescriptorHandle dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
};

struct LTN_GFX_API CommandSignature {
	virtual void initialize(const CommandSignatureDesc& desc) = 0;
	virtual void terminate() = 0;
};

struct LTN_GFX_API Resource {
	virtual void terminate() = 0;
	virtual void* map(const MemoryRange* range) = 0;
	virtual void unmap(const MemoryRange* range = nullptr) = 0;
	virtual u64 getGpuVirtualAddress() const = 0;
	virtual void setDebugName(const char* name) = 0;
};

struct LTN_GFX_API PipelineState {
	virtual void iniaitlize(const GraphicsPipelineStateDesc& desc) = 0;
#if ENABLE_MESH_SHADER
	virtual void iniaitlize(const MeshPipelineStateDesc& desc) = 0;
#endif
	virtual void iniaitlize(const ComputePipelineStateDesc& desc) = 0;
	virtual void terminate() = 0;
	virtual void setDebugName(const char* name) = 0;
	virtual const char* getDebugName() const = 0;
};
struct LTN_GFX_API RootSignature {
	virtual void iniaitlize(const RootSignatureDesc& desc) = 0;
	virtual void terminate() = 0;
	virtual void setDebugName(const char* name) = 0;
	virtual const char* getDebugName() const = 0;
};

class LTN_GFX_API QueryHeap {
public:
	virtual void initialize(const QueryHeapDesc& desc) = 0;
	virtual void terminate() = 0;
};

class LTN_GFX_API GraphicsApiInstanceAllocator {
public:
	virtual HardwareFactory* allocateHardwareFactroy() = 0;
	virtual HardwareAdapter* allocateHardwareAdapter() = 0;
	virtual Device* allocateDevice() = 0;
	virtual SwapChain* allocateSwapChain() = 0;
	virtual CommandQueue* allocateCommandQueue() = 0;
	virtual CommandList* allocateCommandList(u64 fenceValue) = 0;
	virtual Resource* allocateResource() = 0;
	virtual DescriptorHeap* allocateDescriptorHeap() = 0;
	virtual ShaderBlob* allocateShaderBlob() = 0;
	virtual PipelineState* allocatePipelineState() = 0;
	virtual RootSignature* allocateRootSignature() = 0;
	virtual CommandSignature* allocateCommandSignature() = 0;
	virtual QueryHeap* allocateQueryHeap() = 0;

	virtual void initialize() = 0;
	virtual void terminate() = 0;
	virtual void update() = 0;

	static GraphicsApiInstanceAllocator* Get();
};

bool LTN_GFX_API IsSupportedMeshShader(Device* device);
u32 LTN_GFX_API GetConstantBufferAligned(u32 sizeInByte);
u32 LTN_GFX_API GetTextureBufferAligned(u32 sizeInByte);
u8 LTN_GFX_API GetFormatPlaneCount(Device* device, Format format);

constexpr u32 BACK_BUFFER_COUNT = 3;
constexpr Format BACK_BUFFER_FORMAT = FORMAT_R8G8B8A8_UNORM;

namespace DebugMarker {
	constexpr u32 SET_NAME_LENGTH_COUNT_MAX = 128;
	class LTN_GFX_API ScopedEvent {
	public:
		ScopedEvent() {}
		ScopedEvent(CommandList* commandList, const Color4& color, const char* name, ...);
		~ScopedEvent();

		void setEvent(CommandList* commandList, const Color4& color, const char* name, va_list va);
	private:
		CommandList* _commandList = nullptr;
	};

	void LTN_GFX_API setMarker(CommandList* commandList, const Color4& color, const char* name, ...);
	void LTN_GFX_API pushMarker(CommandList* commandList, const Color4& color, const char* name, ...);
	void LTN_GFX_API popMarker(CommandList* commandList);
}

namespace DebugGui {
	using ColorEditFlags = s32;

	enum ColorChannelFilter {
		COLOR_CHANNEL_FILTER_DEFAULT = 0,
		COLOR_CHANNEL_FILTER_R,
		COLOR_CHANNEL_FILTER_G,
		COLOR_CHANNEL_FILTER_B,
		COLOR_CHANNEL_FILTER_RGB,
		COLOR_CHANNEL_FILTER_LINER_DEPTH,
		COLOR_CHANNEL_FILTER_COUNT
	};

	enum TextureSmplerType {
		TEXTURE_SAMPLE_TYPE_LINER = 0,
		TEXTURE_SAMPLE_TYPE_POINT,
		TEXTURE_SAMPLE_TYPE_COUNT
	};

	// ImDrawCornerFlags_
	enum DrawCornerFlags {
		DrawCornerFlags_None = 0,
		DrawCornerFlags_TopLeft = 1 << 0, // 0x1
		DrawCornerFlags_TopRight = 1 << 1, // 0x2
		DrawCornerFlags_BotLeft = 1 << 2, // 0x4
		DrawCornerFlags_BotRight = 1 << 3, // 0x8
		DrawCornerFlags_Top = DrawCornerFlags_TopLeft | DrawCornerFlags_TopRight,   // 0x3
		DrawCornerFlags_Bot = DrawCornerFlags_BotLeft | DrawCornerFlags_BotRight,   // 0xC
		DrawCornerFlags_Left = DrawCornerFlags_TopLeft | DrawCornerFlags_BotLeft,    // 0x5
		DrawCornerFlags_Right = DrawCornerFlags_TopRight | DrawCornerFlags_BotRight,  // 0xA
		DrawCornerFlags_All = 0xF     // In your function calls you may use ~0 (= all bits sets) instead of DrawCornerFlags_All, as a convenience
	};

	struct DebugWindowDesc {
		Device* _device = nullptr;
		s32* _hWnd = nullptr;
		Format _rtvFormat;
		u32 _bufferingCount = 0;
		DescriptorHeap* _descriptorHeap = nullptr;
		DescriptorHandle _srvHandle;
	};

	void LTN_GFX_API InitializeDebugWindowGui(const DebugWindowDesc& desc);
	void LTN_GFX_API BeginDebugWindowGui();
	void LTN_GFX_API RenderDebugWindowImgui();
	void LTN_GFX_API RenderDebugWindowGui(CommandList* commandList);
	bool LTN_GFX_API TranslateWindowProc(s32* hWnd, u32 message, u64 wParam, s64 lParam);
	void LTN_GFX_API Start(const char* windowName);
	void LTN_GFX_API End();
	void LTN_GFX_API TerminateDebugWindowGui();

	void LTN_GFX_API Text(const char* text, ...);
	void LTN_GFX_API TextColored(const Color4& col, const char* fmt, ...);
	void LTN_GFX_API TextDisabled(const char* fmt, ...);

	bool LTN_GFX_API DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);

	bool LTN_GFX_API SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d");

	bool LTN_GFX_API SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);
	bool LTN_GFX_API SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg");

	bool LTN_GFX_API ColorEdit3(const char* label, float col[3], ColorEditFlags flags = 0);
	bool LTN_GFX_API ColorEdit4(const char* label, float col[4], ColorEditFlags flags = 0);

	bool LTN_GFX_API Button(const char* label, const Vector2& size = Vector2(0,0));
	bool LTN_GFX_API Combo(const char* label, s32* current_item, const char* const items[], s32 items_count, s32 popup_max_height_in_items = -1);
	bool LTN_GFX_API Checkbox(const char* label, bool* v);
	bool LTN_GFX_API BeginTabBar(const char* str_id, s32 flags = 0);
	void LTN_GFX_API EndTabBar();
	bool LTN_GFX_API BeginTabItem(const char* label, bool* p_open = nullptr, s32 flags = 0);
	void LTN_GFX_API EndTabItem();
	void LTN_GFX_API Columns(int count = 1, const char* id = nullptr, bool border = true);
	bool LTN_GFX_API TreeNode(const char* label);
	bool LTN_GFX_API TreeNode(s32 id, const char* fmt, ...);
	void LTN_GFX_API TreePop();
	void LTN_GFX_API NextColumn();
	void LTN_GFX_API Separator();
	void LTN_GFX_API SameLine(f32 offsetFromStartX = 0.0f, f32 spacing = -1.0f);
	void LTN_GFX_API SetColumnWidth(s32 columnIndex, f32 width);
	void LTN_GFX_API Image(GpuDescriptorHandle user_texture_id, const Vector2& size, const Vector2& uv0 = Vector2(0, 0), const Vector2& uv1 = Vector2(1, 1),
		const Color4& tint_col = Color4::WHITE, const Color4& border_col = Color4::BLACK,
		ColorChannelFilter channel = COLOR_CHANNEL_FILTER_DEFAULT, const Vector2& colorRange = Vector2(0, 1), TextureSmplerType samplerType = TEXTURE_SAMPLE_TYPE_LINER);
	void LTN_GFX_API ProgressBar(float fraction, const Vector2& sizeArg = Vector2(-1, 0), const char* overlay = nullptr);
	void LTN_GFX_API AddRectFilled(const Vector2& min, const Vector2& max, const Color4& col, f32 rounding = 0.0f, DrawCornerFlags roundingCorners = DrawCornerFlags_All);

	f32 LTN_GFX_API GetColumnWidth(s32 columnIndex = -1);
	Vector2 LTN_GFX_API GetItemInnerSpacing();
	Vector2 LTN_GFX_API GetCursorScreenPos();
}