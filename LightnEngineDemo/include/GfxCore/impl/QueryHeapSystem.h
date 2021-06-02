#pragma once
#include <Core/System.h>
#include <GfxCore/GfxModuleSettings.h>
#include <GfxCore/impl/GpuResourceImpl.h>

struct PerfInfo {
public:
	static constexpr u32 NEST_LEVEL_COUNT_MAX = 16;
	static constexpr u32 TIME_STAMP_COUNT_MAX = 64;
	static constexpr u32 TICK_COUNT_MAX = TIME_STAMP_COUNT_MAX * 2; // begin And end
	static constexpr u32 DEBUG_MARKER_NAME_COUNT_MAX = 128;
	const Vector2 _perfBarOffset = Vector2(12, 0);
	const Vector2 _perfBarSize = Vector2(150, 15);
	const f32 _perfBarRectScale = 0.2f;

	struct TickInfo {
		u16 _beginTickrIndex = 0;
		u16 _endTickIndex = 0;
	};

	u32 pushTick(const char* markerName);
	u32 pushMarker(u32 tickIndex);
	u32 popMarker(u32 tickIndex);

	void reset();
	void debugDrawTree(u32 tickIndex);
	void debugDrawFlat();

	u64 _ticks[TICK_COUNT_MAX] = {};
	f32 _msFrequency = 0;
	u32 _currentFrameMarkerCount = 0;
	u32 _currentTickCount = 0;
	u32 _currentNestLevel = 0;
	u32 _currentNestMarkerIndices[NEST_LEVEL_COUNT_MAX] = {};
	u32 _markerParentIndices[TIME_STAMP_COUNT_MAX] = {};
	u32 _markerNestLevelIndices[TIME_STAMP_COUNT_MAX] = {};
	TickInfo _tickInfos[TIME_STAMP_COUNT_MAX] = {};
	char _markerNames[TIME_STAMP_COUNT_MAX][DEBUG_MARKER_NAME_COUNT_MAX] = {};
};

class LTN_GFX_CORE_API QueryHeapSystem {
public:
	static constexpr u32 NEST_COUNT_MAX = 16;
	static constexpr u32 GPU_TIME_STAMP_COUNT_MAX = 64;
	static constexpr u32 TICK_COUNT_MAX = GPU_TIME_STAMP_COUNT_MAX * 2; // begin end
	static constexpr u32 DEBUG_MARKER_NAME_COUNT_MAX = 128;

	void initialize();
	void terminate();
	void update();
	void requestTimeStamp(CommandList* commandList, u32 frameIndex);
	void setGpuFrequency(CommandQueue* commandQueue);
	void setCpuFrequency();
	void debugDrawTimeStamps();
	void debugDrawCpuPerf();
	void debugDrawGpuPerf();

	u32 pushGpuMarker(CommandList* commandList, const char* markerName);
	void popGpuMarker(CommandList* commandList, u32 markerIndex);
	u32 pushCpuMarker(const char* markerName);
	void popCpuMarker(u32 markerIndex);

	f32 getCurrentCpuFrameTime() const;
	f32 getCurrentGpuFrameTime() const;

	static QueryHeapSystem* Get();

private:
	GpuBuffer _timeStampBuffer;
	QueryHeap* _queryHeap = nullptr;

	PerfInfo _gpuPerf;
	PerfInfo _cpuPerf;
};

class LTN_GFX_CORE_API GpuScopedEvent {
public:
	GpuScopedEvent(CommandList* commandList, const Color4& color, const char* name, ...) {
		_commandList = commandList;
		va_list va;
		va_start(va, name);
		vsprintf_s(_name, name, va);
		_gpuMarker.setEvent(commandList, color, name, va);
		va_end(va);

		QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
		_markerIndex = queryHeapSystem->pushGpuMarker(_commandList, _name);
	}
	~GpuScopedEvent() {
		QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
		queryHeapSystem->popGpuMarker(_commandList, _markerIndex);
	}
private:
	DebugMarker::ScopedEvent _gpuMarker;
	u32 _markerIndex = 0;
	CommandList* _commandList = nullptr;
	char _name[DebugMarker::SET_NAME_LENGTH_COUNT_MAX] = {};
};

class LTN_GFX_CORE_API CpuScopedEvent {
public:
	CpuScopedEvent(const char* name, ...) {
		va_list va;
		va_start(va, name);
		vsprintf_s(_name, name, va);
		va_end(va);

		QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
		_cpuTickIndex = queryHeapSystem->pushCpuMarker(_name);
	}
	~CpuScopedEvent() {
		QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
		queryHeapSystem->popCpuMarker(_cpuTickIndex);
	}
private:
	u32 _cpuTickIndex = 0;
	char _name[DebugMarker::SET_NAME_LENGTH_COUNT_MAX] = {};
};

class LTN_GFX_CORE_API CpuGpuScopedEvent {
public:
	CpuGpuScopedEvent(CommandList* commandList, const Color4& color, const char* name, ...) {
		_commandList = commandList;
		va_list va;
		va_start(va, name);
		vsprintf_s(_name, name, va);
		_gpuMarker.setEvent(commandList, color, name, va);
		va_end(va);

		QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
		_gpuTickIndex = queryHeapSystem->pushGpuMarker(_commandList, _name);
		_cpuTickIndex = queryHeapSystem->pushCpuMarker(_name);
	}
	~CpuGpuScopedEvent() {
		QueryHeapSystem* queryHeapSystem = QueryHeapSystem::Get();
		queryHeapSystem->popGpuMarker(_commandList, _gpuTickIndex);
		queryHeapSystem->popCpuMarker(_cpuTickIndex);
	}
private:
	DebugMarker::ScopedEvent _gpuMarker;
	CommandList* _commandList = nullptr;
	u32 _gpuTickIndex = 0;
	u32 _cpuTickIndex = 0;
	char _name[DebugMarker::SET_NAME_LENGTH_COUNT_MAX] = {};
};

#define DEBUG_MARKER_CPU_SCOPED_EVENT(...) CpuScopedEvent __DEBUG_SCOPED_EVENT__(__VA_ARGS__)
#define DEBUG_MARKER_GPU_SCOPED_EVENT(...) GpuScopedEvent __DEBUG_SCOPED_EVENT__(__VA_ARGS__)
#define DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(...) CpuGpuScopedEvent __DEBUG_SCOPED_EVENT__(__VA_ARGS__)