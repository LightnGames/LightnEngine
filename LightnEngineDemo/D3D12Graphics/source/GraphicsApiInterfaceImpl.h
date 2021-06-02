#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
#include <fstream>

// メッシュシェーダー用の仮の定義です
// 正式版がリリースされたら正しいAPIに置き換えます
namespace {
	struct CD3DX12_DEFAULT {};
	extern const DECLSPEC_SELECTANY CD3DX12_DEFAULT D3D12_DEFAULT;

	struct DefaultSampleMask { operator UINT() noexcept { return UINT_MAX; } };
	struct DefaultSampleDesc { operator DXGI_SAMPLE_DESC() noexcept { return DXGI_SAMPLE_DESC{ 1, 0 }; } };

	//------------------------------------------------------------------------------------------------
	struct CD3DX12_DEPTH_STENCIL_DESC : public D3D12_DEPTH_STENCIL_DESC {
		CD3DX12_DEPTH_STENCIL_DESC() = default;
		explicit CD3DX12_DEPTH_STENCIL_DESC(const D3D12_DEPTH_STENCIL_DESC& o) noexcept :
			D3D12_DEPTH_STENCIL_DESC(o) {
		}
		explicit CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT) noexcept {
			DepthEnable = TRUE;
			DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			StencilEnable = FALSE;
			StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
			StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
			const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
			{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
			FrontFace = defaultStencilOp;
			BackFace = defaultStencilOp;
		}
		explicit CD3DX12_DEPTH_STENCIL_DESC(
			BOOL depthEnable,
			D3D12_DEPTH_WRITE_MASK depthWriteMask,
			D3D12_COMPARISON_FUNC depthFunc,
			BOOL stencilEnable,
			UINT8 stencilReadMask,
			UINT8 stencilWriteMask,
			D3D12_STENCIL_OP frontStencilFailOp,
			D3D12_STENCIL_OP frontStencilDepthFailOp,
			D3D12_STENCIL_OP frontStencilPassOp,
			D3D12_COMPARISON_FUNC frontStencilFunc,
			D3D12_STENCIL_OP backStencilFailOp,
			D3D12_STENCIL_OP backStencilDepthFailOp,
			D3D12_STENCIL_OP backStencilPassOp,
			D3D12_COMPARISON_FUNC backStencilFunc) noexcept {
			DepthEnable = depthEnable;
			DepthWriteMask = depthWriteMask;
			DepthFunc = depthFunc;
			StencilEnable = stencilEnable;
			StencilReadMask = stencilReadMask;
			StencilWriteMask = stencilWriteMask;
			FrontFace.StencilFailOp = frontStencilFailOp;
			FrontFace.StencilDepthFailOp = frontStencilDepthFailOp;
			FrontFace.StencilPassOp = frontStencilPassOp;
			FrontFace.StencilFunc = frontStencilFunc;
			BackFace.StencilFailOp = backStencilFailOp;
			BackFace.StencilDepthFailOp = backStencilDepthFailOp;
			BackFace.StencilPassOp = backStencilPassOp;
			BackFace.StencilFunc = backStencilFunc;
		}
	};

	//------------------------------------------------------------------------------------------------
	struct CD3DX12_DEPTH_STENCIL_DESC1 : public D3D12_DEPTH_STENCIL_DESC1 {
		CD3DX12_DEPTH_STENCIL_DESC1() = default;
		explicit CD3DX12_DEPTH_STENCIL_DESC1(const D3D12_DEPTH_STENCIL_DESC1& o) noexcept :
			D3D12_DEPTH_STENCIL_DESC1(o) {
		}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(const D3D12_DEPTH_STENCIL_DESC& o) noexcept {
			DepthEnable = o.DepthEnable;
			DepthWriteMask = o.DepthWriteMask;
			DepthFunc = o.DepthFunc;
			StencilEnable = o.StencilEnable;
			StencilReadMask = o.StencilReadMask;
			StencilWriteMask = o.StencilWriteMask;
			FrontFace.StencilFailOp = o.FrontFace.StencilFailOp;
			FrontFace.StencilDepthFailOp = o.FrontFace.StencilDepthFailOp;
			FrontFace.StencilPassOp = o.FrontFace.StencilPassOp;
			FrontFace.StencilFunc = o.FrontFace.StencilFunc;
			BackFace.StencilFailOp = o.BackFace.StencilFailOp;
			BackFace.StencilDepthFailOp = o.BackFace.StencilDepthFailOp;
			BackFace.StencilPassOp = o.BackFace.StencilPassOp;
			BackFace.StencilFunc = o.BackFace.StencilFunc;
			DepthBoundsTestEnable = FALSE;
		}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(CD3DX12_DEFAULT) noexcept {
			DepthEnable = TRUE;
			DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			StencilEnable = FALSE;
			StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
			StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
			const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
			{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
			FrontFace = defaultStencilOp;
			BackFace = defaultStencilOp;
			DepthBoundsTestEnable = FALSE;
}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(
			BOOL depthEnable,
			D3D12_DEPTH_WRITE_MASK depthWriteMask,
			D3D12_COMPARISON_FUNC depthFunc,
			BOOL stencilEnable,
			UINT8 stencilReadMask,
			UINT8 stencilWriteMask,
			D3D12_STENCIL_OP frontStencilFailOp,
			D3D12_STENCIL_OP frontStencilDepthFailOp,
			D3D12_STENCIL_OP frontStencilPassOp,
			D3D12_COMPARISON_FUNC frontStencilFunc,
			D3D12_STENCIL_OP backStencilFailOp,
			D3D12_STENCIL_OP backStencilDepthFailOp,
			D3D12_STENCIL_OP backStencilPassOp,
			D3D12_COMPARISON_FUNC backStencilFunc,
			BOOL depthBoundsTestEnable) noexcept {
			DepthEnable = depthEnable;
			DepthWriteMask = depthWriteMask;
			DepthFunc = depthFunc;
			StencilEnable = stencilEnable;
			StencilReadMask = stencilReadMask;
			StencilWriteMask = stencilWriteMask;
			FrontFace.StencilFailOp = frontStencilFailOp;
			FrontFace.StencilDepthFailOp = frontStencilDepthFailOp;
			FrontFace.StencilPassOp = frontStencilPassOp;
			FrontFace.StencilFunc = frontStencilFunc;
			BackFace.StencilFailOp = backStencilFailOp;
			BackFace.StencilDepthFailOp = backStencilDepthFailOp;
			BackFace.StencilPassOp = backStencilPassOp;
			BackFace.StencilFunc = backStencilFunc;
			DepthBoundsTestEnable = depthBoundsTestEnable;
		}
		operator D3D12_DEPTH_STENCIL_DESC() const noexcept {
			D3D12_DEPTH_STENCIL_DESC D;
			D.DepthEnable = DepthEnable;
			D.DepthWriteMask = DepthWriteMask;
			D.DepthFunc = DepthFunc;
			D.StencilEnable = StencilEnable;
			D.StencilReadMask = StencilReadMask;
			D.StencilWriteMask = StencilWriteMask;
			D.FrontFace.StencilFailOp = FrontFace.StencilFailOp;
			D.FrontFace.StencilDepthFailOp = FrontFace.StencilDepthFailOp;
			D.FrontFace.StencilPassOp = FrontFace.StencilPassOp;
			D.FrontFace.StencilFunc = FrontFace.StencilFunc;
			D.BackFace.StencilFailOp = BackFace.StencilFailOp;
			D.BackFace.StencilDepthFailOp = BackFace.StencilDepthFailOp;
			D.BackFace.StencilPassOp = BackFace.StencilPassOp;
			D.BackFace.StencilFunc = BackFace.StencilFunc;
			return D;
		}
	};

	//------------------------------------------------------------------------------------------------
	struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC {
		CD3DX12_BLEND_DESC() = default;
		explicit CD3DX12_BLEND_DESC(const D3D12_BLEND_DESC& o) noexcept :
			D3D12_BLEND_DESC(o) {
		}
		explicit CD3DX12_BLEND_DESC(CD3DX12_DEFAULT) noexcept {
			AlphaToCoverageEnable = FALSE;
			IndependentBlendEnable = FALSE;
			const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
			{
				FALSE,FALSE,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				RenderTarget[i] = defaultRenderTargetBlendDesc;
		}
	};


	//------------------------------------------------------------------------------------------------
	struct CD3DX12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC {
		CD3DX12_RASTERIZER_DESC() = default;
		explicit CD3DX12_RASTERIZER_DESC(const D3D12_RASTERIZER_DESC& o) noexcept :
			D3D12_RASTERIZER_DESC(o) {
		}
		explicit CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT) noexcept {
			FillMode = D3D12_FILL_MODE_SOLID;
			CullMode = D3D12_CULL_MODE_BACK;
			FrontCounterClockwise = FALSE;
			DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			DepthClipEnable = TRUE;
			MultisampleEnable = FALSE;
			AntialiasedLineEnable = FALSE;
			ForcedSampleCount = 0;
			ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		}
		explicit CD3DX12_RASTERIZER_DESC(
			D3D12_FILL_MODE fillMode,
			D3D12_CULL_MODE cullMode,
			BOOL frontCounterClockwise,
			INT depthBias,
			FLOAT depthBiasClamp,
			FLOAT slopeScaledDepthBias,
			BOOL depthClipEnable,
			BOOL multisampleEnable,
			BOOL antialiasedLineEnable,
			UINT forcedSampleCount,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster) noexcept {
			FillMode = fillMode;
			CullMode = cullMode;
			FrontCounterClockwise = frontCounterClockwise;
			DepthBias = depthBias;
			DepthBiasClamp = depthBiasClamp;
			SlopeScaledDepthBias = slopeScaledDepthBias;
			DepthClipEnable = depthClipEnable;
			MultisampleEnable = multisampleEnable;
			AntialiasedLineEnable = antialiasedLineEnable;
			ForcedSampleCount = forcedSampleCount;
			ConservativeRaster = conservativeRaster;
		}
	};

#if ENABLE_MESH_SHADER
	template <typename InnerStructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type, typename DefaultArg = InnerStructType>
	class alignas(void*) CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT {
	private:
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _Type;
		InnerStructType _Inner;
	public:
		CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT() noexcept : _Type(Type), _Inner(DefaultArg()) {}
		CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT(InnerStructType const& i) noexcept : _Type(Type), _Inner(i) {}
		CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT& operator=(InnerStructType const& i) noexcept { _Type = Type; _Inner = i; return *this; }
		operator InnerStructType const& () const noexcept { return _Inner; }
		operator InnerStructType& () noexcept { return _Inner; }
		InnerStructType* operator&() noexcept { return &_Inner; }
		InnerStructType const* operator&() const noexcept { return &_Inner; }
	};

	static const GUID D3D12ExperimentalShaderModelsID = { /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
	0x76f5573e,
	0xf13a,
	0x40f5,
	{ 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
	};

	//------------------------------------------------------------------------------------------------
	struct CD3DX12_VIEW_INSTANCING_DESC : public D3D12_VIEW_INSTANCING_DESC {
		CD3DX12_VIEW_INSTANCING_DESC() = default;
		explicit CD3DX12_VIEW_INSTANCING_DESC(const D3D12_VIEW_INSTANCING_DESC& o) noexcept :
			D3D12_VIEW_INSTANCING_DESC(o) {
		}
		explicit CD3DX12_VIEW_INSTANCING_DESC(CD3DX12_DEFAULT) noexcept {
			ViewInstanceCount = 0;
			pViewInstanceLocations = nullptr;
			Flags = D3D12_VIEW_INSTANCING_FLAG_NONE;
		}
		explicit CD3DX12_VIEW_INSTANCING_DESC(
			UINT InViewInstanceCount,
			const D3D12_VIEW_INSTANCE_LOCATION* InViewInstanceLocations,
			D3D12_VIEW_INSTANCING_FLAGS InFlags) noexcept {
			ViewInstanceCount = InViewInstanceCount;
			pViewInstanceLocations = InViewInstanceLocations;
			Flags = InFlags;
		}
	};

	// Create a pipeline state stream descriptor for the mesh shader pipeline.
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_PIPELINE_STATE_FLAGS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS>                             CD3DX12_PIPELINE_STATE_STREAM_FLAGS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< UINT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>                         CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>                    CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_INPUT_LAYOUT_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>                      CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_INDEX_BUFFER_STRIP_CUT_VALUE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE>                CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>                CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>                                CD3DX12_PIPELINE_STATE_STREAM_VS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>                                CD3DX12_PIPELINE_STATE_STREAM_GS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_STREAM_OUTPUT_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT>                     CD3DX12_PIPELINE_STATE_STREAM_STREAM_OUTPUT;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>                                CD3DX12_PIPELINE_STATE_STREAM_PS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>                                CD3DX12_PIPELINE_STATE_STREAM_AS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>                                CD3DX12_PIPELINE_STATE_STREAM_MS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< CD3DX12_BLEND_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, CD3DX12_DEFAULT>   CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< CD3DX12_DEPTH_STENCIL_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, CD3DX12_DEFAULT>   CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< CD3DX12_DEPTH_STENCIL_DESC1, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, CD3DX12_DEFAULT>   CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>              CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< CD3DX12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, CD3DX12_DEFAULT>   CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>             CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< DXGI_SAMPLE_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DefaultSampleDesc> CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< UINT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, DefaultSampleMask> CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_CACHED_PIPELINE_STATE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO>                        CD3DX12_PIPELINE_STATE_STREAM_CACHED_PSO;
	typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< CD3DX12_VIEW_INSTANCING_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING, CD3DX12_DEFAULT>  CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING;

	struct MeshShaderPsoDesc {
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_AS                    AS;
		CD3DX12_PIPELINE_STATE_STREAM_MS                    MS;
		CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DepthFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
	};
#endif
}

enum GraphicsInterfaceStateFlags {
	GRAPHICS_INTERFACE_STATE_NONE = 0,
	GRAPHICS_INTERFACE_STATE_ENABLE = 1,
	GRAPHICS_INTERFACE_STATE_REQUEST_DELETE = 2
};

struct ShaderBlobD3D12 :public ShaderBlob {
	virtual void initialize(const char* filePath) override;
	virtual void terminate() override;
	virtual ShaderByteCode getShaderByteCode() const override;

	ID3DBlob* _blob = nullptr;
	u8* _stateFlags = nullptr;
};

struct HardwareFactoryD3D12 :public HardwareFactory {
	virtual void initialize(const HardwareFactoryDesc& desc) override;
	virtual void terminate() override;

	IDXGIFactory4* _factory = nullptr;
	u8* _stateFlags = nullptr;
};

struct HardwareAdapterD3D12 :public HardwareAdapter {
	virtual void initialize(const HardwareAdapterDesc& desc) override;
	virtual void terminate() override;
	virtual QueryVideoMemoryInfo queryVideoMemoryInfo() override;

	IDXGIAdapter3* _adapter = nullptr;
	u8* _stateFlags = nullptr;
};

struct DeviceD3D12 :public Device {
	virtual void initialize(const DeviceDesc& desc) override;
	virtual void terminate() override;
	virtual u32 getDescriptorHandleIncrementSize(DescriptorHeapType type) const override;
	virtual void createRenderTargetView(Resource* resource, CpuDescriptorHandle destDescriptor) override;
	virtual void createDepthStencilView(Resource* resource, CpuDescriptorHandle destDescriptor) override;
	virtual void createCommittedResource(HeapType heapType, HeapFlags heapFlags, const ResourceDesc& desc,
		ResourceStates initialResourceState, const ClearValue* optimizedClearValue, Resource* dstResource) override;
	virtual void createConstantBufferView(const ConstantBufferViewDesc& desc, CpuDescriptorHandle destDescriptor) override;
	virtual void createShaderResourceView(Resource* resource, const ShaderResourceViewDesc* desc, CpuDescriptorHandle destDescriptor) override;
	virtual void createUnorderedAccessView(Resource* resource, Resource* counterResource, const UnorderedAccessViewDesc* desc, CpuDescriptorHandle destDescriptor) override;
	virtual void getCopyableFootprints(const ResourceDesc* resourceDesc, u32 firstSubresource, u32 numSubresources, u64 baseOffset, PlacedSubresourceFootprint* layouts, u32* numRows, u64* rowSizeInBytes, u64* totalBytes) override;
	virtual u8 getFormatPlaneCount(Format format) override;

	ID3D12Device2* _device = nullptr;
	u8* _stateFlags = nullptr;
};

struct SwapChainD3D12 :public SwapChain {
	virtual void initialize(const SwapChainDesc& desc) override;
	virtual void terminate() override;

	virtual u32 getCurrentBackBufferIndex() override;
	virtual void present(u32 syncInterval, u32 flags) override;
	virtual void getBackBuffer(Resource** resource, u32 index) override;

	IDXGISwapChain3* _swapChain = nullptr;
	u8* _stateFlags = nullptr;
};

struct CommandQueueD3D12 :public CommandQueue {
	virtual void initialize(const CommandQueueDesc& desc) override;
	virtual void terminate() override;
	virtual void executeCommandLists(u32 count, CommandList** commandLists) override;
	virtual void getTimestampFrequency(u64* frequency) override;

	bool isFenceComplete(u64 fenceValue);
	void waitForIdle();
	void waitForFence(u64 fenceValue) override;
	u64 incrimentFence() override;
	u64 getCompleatedValue() const override;

	ID3D12CommandQueue* _commandQueue = nullptr;
	ID3D12Fence* _fence = nullptr;
	HANDLE _fenceEvent = nullptr;
	u64 _nextFenceValue = 0;
	u64 _lastFenceValue = 0;
	u8* _stateFlags = nullptr;
};

struct DescriptorHeapD3D12 :public DescriptorHeap {
	virtual void initialize(const DescriptorHeapDesc& desc) override;
	virtual void terminate() override;
	virtual u32 getIncrimentSize() const override;
	virtual CpuDescriptorHandle getCPUDescriptorHandleForHeapStart() const override;
	virtual GpuDescriptorHandle getGPUDescriptorHandleForHeapStart() const override;

	ID3D12DescriptorHeap* _descriptorHeap = nullptr;
	u8* _stateFlags = nullptr;
};

struct CommandListD3D12 : public CommandList {
	static constexpr u64 INVAILD_FENCE_VALUE = static_cast<u64>(-1);
	virtual void initialize(const CommandListDesc& desc) override;
	virtual void terminate() override;

	virtual void transitionBarrierSimple(Resource* resource, ResourceStates currentState, ResourceStates nextState) override;
	virtual void transitionBarriers(ResourceTransitionBarrier* barriers, u32 count) override;
	virtual void copyBufferRegion(Resource* dstBuffer, u64 dstOffset, Resource* srcBuffer, u64 srcOffset, u64 numBytes) override;
	virtual void copyTextureRegion(const TextureCopyLocation* dst, u32 dstX, u32 dstY, u32 dstZ, const TextureCopyLocation* src, const Box* srcBox) override;
	virtual void copyResource(Resource* dstResource, Resource* srcResource) override;
	virtual void setDescriptorHeaps(u32 count, DescriptorHeap** descriptorHeaps) override;
	virtual void setViewports(u32 count, const ViewPort* viewPorts) override;
	virtual void setScissorRects(u32 count, const Rect* scissorRects) override;
	virtual void setRenderTargets(u32 count, DescriptorHandle* rtvHandles, DescriptorHandle* dsvHandle) override;
	virtual void clearRenderTargetView(DescriptorHandle rtvHandle, f32 clearColor[4]) override;
	virtual void clearDepthStencilView(CpuDescriptorHandle depthStencilView, ClearFlags clearFlags, f32 depth, u8 stencil, u32 numRects, const Rect* rects) override;
	virtual void setPipelineState(PipelineState* pipelineState) override;
	virtual void setGraphicsRootSignature(RootSignature* rootSignature) override;
	virtual void setComputeRootSignature(RootSignature* rootSignature) override;
	virtual void setPrimitiveTopology(PrimitiveTopology primitiveTopology) override;
	virtual void setGraphicsRootDescriptorTable(u32 rootParameterIndex, GpuDescriptorHandle baseDescriptor) override;
	virtual void setComputeRootDescriptorTable(u32 rootParameterIndex, GpuDescriptorHandle baseDescriptor) override;
	virtual void setGraphicsRoot32BitConstants(u32 rootParameterIndex, u32 num32BitValuesToSet, const void* srcData, u32 destOffsetIn32BitValues) override;
	virtual void drawInstanced(u32 vertexCountPerInstance, u32 instanceCount, u32 startVertexLocation, u32 startInstanceLocation) override;
	virtual void drawIndexedInstanced(u32 indexCountPerInstance, u32 instanceCount, u32 startIndexLocation, s32 baseVertexLocation, u32 startInstanceLocation) override;
	virtual void dispatch(u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) override;
	virtual void executeIndirect(CommandSignature* commandSignature, u32 maxCommandCount, Resource* argumentBuffer, u64 argumentBufferOffset, Resource* countBuffer, u64 countBufferOffset) override;
	virtual void clearUnorderedAccessViewUint(GpuDescriptorHandle viewGPUHandleInCurrentHeap, CpuDescriptorHandle viewCPUHandle, Resource* resource, const u32 values[4], u32 numRects, const Rect* rects) override;
	virtual void setVertexBuffers(u32 startSlot, u32 numViews, const VertexBufferView* views) override;
	virtual void setIndexBuffer(const IndexBufferView* view) override;
	virtual void endQuery(QueryHeap* queryHeap, QueryType type, u32 index) override;
	virtual void resolveQueryData(QueryHeap* queryHeap, QueryType type, u32 startIndex, u32 numQueries, Resource* destinationBuffer, u64 alignedDestinationBufferOffset) override;
#if ENABLE_MESH_SHADER
	virtual void dispatchMesh(u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) override;
#endif

	u64* _fenceValue = nullptr;
	ID3D12CommandAllocator* _allocator = nullptr;
#if ENABLE_MESH_SHADER
	ID3D12GraphicsCommandList6* _commandList = nullptr;
#else
	ID3D12GraphicsCommandList* _commandList = nullptr;
#endif
	u8* _stateFlags = nullptr;
};

struct CommandSignatureD3D12 :public CommandSignature {
	virtual void initialize(const CommandSignatureDesc& desc) override;
	virtual void terminate() override;
	ID3D12CommandSignature* _commandSignature = nullptr;
	u8* _stateFlags = nullptr;
};

struct ResourceD3D12 :public Resource {
	void initialize(ID3D12Resource* resource) { _resource = resource; }
	virtual void terminate() override;
	virtual void* map(const MemoryRange* range) override;
	virtual void unmap(const MemoryRange* range) override;
	virtual u64 getGpuVirtualAddress() const override;
	virtual void setDebugName(const char* name) override;
	ID3D12Resource* _resource = nullptr;
	u8* _stateFlags = nullptr;
};

struct PipelineStateD3D12 : public PipelineState {
	virtual void iniaitlize(const GraphicsPipelineStateDesc& desc) override;
#if ENABLE_MESH_SHADER
	virtual void iniaitlize(const MeshPipelineStateDesc& desc) override;
#endif
	virtual void iniaitlize(const ComputePipelineStateDesc& desc) override;
	virtual void terminate() override;
	virtual void setDebugName(const char* name) override;
	virtual const char* getDebugName() const override { return _debugName; }
	ID3D12PipelineState* _pipelineState = nullptr;
	const char* _debugName = nullptr;
	u8* _stateFlags = nullptr;
};

struct RootSignatureD3D12 : public RootSignature {
	virtual void iniaitlize(const RootSignatureDesc& desc) override;
	virtual void terminate() override;
	virtual void setDebugName(const char* name) override;
	virtual const char* getDebugName() const override { return _debugName; }
	ID3D12RootSignature* _rootSignature = nullptr;
	const char* _debugName = nullptr;
	u8* _stateFlags = nullptr;
};

class QueryHeapD3D12 : public QueryHeap {
public:
	virtual void initialize(const QueryHeapDesc& desc) override;
	virtual void terminate() override;

	u8* _stateFlags = nullptr;
	ID3D12QueryHeap* _queryHeap = nullptr;
};

template<class T, u32 N>
struct GraphicsApiArray {
	void initialize() {
		_array.initialize(N);
	}

	void resetStateFlags(u32 index) {
		_flags[index] = GRAPHICS_INTERFACE_STATE_ENABLE;
		_array[index]._stateFlags = &_flags[index];
	}

	u32 request() {
		u32 index = _array.request();
		resetStateFlags(index);
		return index;
	}

	T* requestData() {
		return &_array[request()];
	}

	void discard(u32 index) {
		_flags[index] = GRAPHICS_INTERFACE_STATE_NONE;
		_array.discard(index);
	}

	void terminate() {
		_array.terminate();
	}

	u32 getInstanceCount() const {
		return _array.getInstanceCount();
	}

	u32 getArrayCountMax() const {
		return _array.getResarveCount();
	}

	T& operator [](u32 index) { return _array[index]; }

	u8 _flags[N] = {};
	DynamicQueue<T> _array;
};

class GraphicsApiInstanceAllocatorImpl :public GraphicsApiInstanceAllocator {
public:
	static constexpr u32 HARDWARE_FACTORY_COUNT_MAX = 8;
	static constexpr u32 HARDWARE_ADAPTER_COUNT_MAX = 8;
	static constexpr u32 DEVICE_COUNT_MAX = 4;
	static constexpr u32 SWAPCHAIN_COUNT_MAX = 4;
	static constexpr u32 COMMAND_QUEUE_COUNT_MAX = 8;
	static constexpr u32 COMMAND_LIST_COUNT_MAX = 32;
	static constexpr u32 RESOURCE_COUNT_MAX = 512;
	static constexpr u32 DESCRIPTOR_HEAP_COUNT_MAX = 8;
	static constexpr u32 SHADER_BLOB_COUNT_MAX = 128;
	static constexpr u32 PIPELINE_STATE_COUNT_MAX = 128;
	static constexpr u32 ROOT_SIGNATURE_COUNT_MAX = 128;
	static constexpr u32 COMMAND_SIGNATURE_COUNT_MAX = 128;
	static constexpr u32 QUERY_HEAP_COUNT_MAX = 4;

	virtual HardwareFactory* allocateHardwareFactroy() override;
	virtual HardwareAdapter* allocateHardwareAdapter() override;
	virtual Device* allocateDevice() override;
	virtual SwapChain* allocateSwapChain() override;
	virtual CommandQueue* allocateCommandQueue() override;
	virtual CommandList* allocateCommandList(u64 fenceValue) override;
	virtual Resource* allocateResource() override;
	virtual DescriptorHeap* allocateDescriptorHeap() override;
	virtual ShaderBlob* allocateShaderBlob() override;
	virtual PipelineState* allocatePipelineState() override;
	virtual RootSignature* allocateRootSignature() override;
	virtual CommandSignature* allocateCommandSignature() override;
	virtual QueryHeap* allocateQueryHeap() override;

	virtual void initialize() override;
	virtual void terminate() override;
	virtual void update() override;

	static GraphicsApiInstanceAllocatorImpl* Get();

private:
	GraphicsApiArray<HardwareFactoryD3D12, HARDWARE_FACTORY_COUNT_MAX> _hardwareFactories;
	GraphicsApiArray<HardwareAdapterD3D12, HARDWARE_ADAPTER_COUNT_MAX> _hardwareAdapters;
	GraphicsApiArray<DeviceD3D12, DEVICE_COUNT_MAX> _devices;
	GraphicsApiArray<SwapChainD3D12, SWAPCHAIN_COUNT_MAX> _swapChains;
	GraphicsApiArray<CommandQueueD3D12, COMMAND_QUEUE_COUNT_MAX> _commandQueues;
	GraphicsApiArray<CommandListD3D12, COMMAND_LIST_COUNT_MAX> _commandLists;
	GraphicsApiArray<ResourceD3D12, RESOURCE_COUNT_MAX> _resources;
	GraphicsApiArray<DescriptorHeapD3D12, DESCRIPTOR_HEAP_COUNT_MAX> _descriptorHeaps;
	GraphicsApiArray<ShaderBlobD3D12, SHADER_BLOB_COUNT_MAX> _shaderBlobs;
	GraphicsApiArray<PipelineStateD3D12, PIPELINE_STATE_COUNT_MAX> _pipelineStates;
	GraphicsApiArray<RootSignatureD3D12, ROOT_SIGNATURE_COUNT_MAX> _rootSignatures;
	GraphicsApiArray<CommandSignatureD3D12, COMMAND_SIGNATURE_COUNT_MAX> _commandSignatures;
	GraphicsApiArray<QueryHeapD3D12, QUERY_HEAP_COUNT_MAX> _queryHeaps;
	u64 _commandListFenceValues[COMMAND_LIST_COUNT_MAX] = {};
};