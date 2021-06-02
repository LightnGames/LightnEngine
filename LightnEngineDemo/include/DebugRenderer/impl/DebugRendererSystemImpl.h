#pragma once
#include <DebugRenderer/DebugRendererSystem.h>
#include <GfxCore/impl/GpuResourceImpl.h>
struct CommandList;
struct PipelineState;
struct RootSignature;
struct ViewInfo;

struct LineInstance {
	Float3 _startPosition;
	Float3 _endPosition;
	Color4 _color;
};

struct BoxInstance {
	Matrix43 _matrixWorld;
	Color4 _color;
};

class LTN_DEBUG_RENDERER_API  DebugRendererSystemImpl : public DebugRendererSystem {
public:
	static const u32 LINE_INSTANCE_CPU_COUNT_MAX = 1024 * 16;
	static const u32 BOX_INSTANCE_CPU_COUNT_MAX = 1024 * 8;
	static const u32 LINE_INSTANCE_GPU_COUNT_MAX = 1024 * 64;
	static const u32 BOX_INSTANCE_GPU_COUNT_MAX = 1024 * 256;

	void initialize();
	void update();
	void processDeletion();
	void terminate();
	void resetGpuCounter(CommandList* commandList);
	void render(CommandList* commandList, const ViewInfo* viewInfo);
	GpuDescriptorHandle getLineGpuUavHandle() const { return _lineInstanceGpuUav._gpuHandle; }
	GpuDescriptorHandle getBoxGpuUavHandle() const { return _boxInstanceGpuUav._gpuHandle; }

	virtual void drawLine(Vector3 startPosition, Vector3 endPosition, Color4 color) override;
	virtual void drawBox(Matrix4 matrix, Color4 color = Color4::RED) override;
	virtual void drawAabb(Vector3 boundsMin, Vector3 boundsMax, Color4 color = Color4::RED) override;
	virtual void drawFrustum(Matrix4 view, Matrix4 projection, Color4 color) override;

	static DebugRendererSystemImpl* Get();

private:
	LinerAllocater<LineInstance> _lineInstances;
	LinerAllocater<BoxInstance> _boxInstances;
	RootSignature* _rootSignature = nullptr;
	PipelineState* _debugLinePipelineState = nullptr;
	PipelineState* _debugBoxPipelineState = nullptr;
	GpuBuffer _lineInstanceCpuBuffer;
	GpuBuffer _lineInstanceGpuBuffer;
	GpuBuffer _boxInstanceCpuBuffer;
	GpuBuffer _boxInstanceGpuBuffer;
	GpuBuffer _lineInstanceIndirectArgumentBuffer;
	GpuBuffer _boxInstanceIndirectArgumentBuffer;
	DescriptorHandle _lineInstanceCpuSrv;
	DescriptorHandle _lineInstanceGpuUav;
	DescriptorHandle _lineInstanceGpuSrv;
	DescriptorHandle _boxInstanceCpuSrv;
	DescriptorHandle _boxInstanceGpuUav;
	DescriptorHandle _boxInstanceGpuSrv;
	CommandSignature* _commandSignature = nullptr;
};