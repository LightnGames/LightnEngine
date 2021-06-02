#include <GfxCore/impl/DebugWindow.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <Core/Application.h>
#include <direct.h>

DescriptorHandle _imguiHandle;

#if DEBUG_WINDOW_ENABLE
#define SWITCH_CODE(x) x
#else
#define SWITCH_CODE(x) true
#endif

void DebugWindow::Start(const char* windowName) {
	DebugGui::Start(windowName);
}

void DebugWindow::initialize() {
#if DEBUG_WINDOW_ENABLE
	GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
	DescriptorHeapAllocator* descriptorHeapAllocater = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator();
	_imguiHandle = descriptorHeapAllocater->allocateDescriptors(1);

	Application* app = ApplicationSystem::Get()->getApplication();
	DebugGui::DebugWindowDesc desc = {};
	desc._device = graphicsSystem->getDevice();
	desc._descriptorHeap = descriptorHeapAllocater->getDescriptorHeap();
	desc._srvHandle = _imguiHandle;
	desc._hWnd = app->getWindowHandle();
	desc._bufferingCount = BACK_BUFFER_COUNT;
	desc._rtvFormat = BACK_BUFFER_FORMAT;
	DebugGui::InitializeDebugWindowGui(desc);

	if (_mkdir(DEBUG_WINDOW_STRUCTURE_FOLDER_PATH)) {
		LTN_INFO("create debug window folder %s", DEBUG_WINDOW_STRUCTURE_FOLDER_PATH);
	}

	s32* hWnd = app->getWindowHandle();
	ApplicationCallBack f = [hWnd](u32 message, u64 wParam, s64 lParam) {
		DebugGui::TranslateWindowProc(hWnd, message, wParam, lParam);
	};
	app->registApplicationCallBack(f);

#endif
}

void DebugWindow::terminate() {
#if DEBUG_WINDOW_ENABLE
	DebugGui::TerminateDebugWindowGui();

	DescriptorHeapAllocator* descriptorHeap = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	descriptorHeap->discardDescriptor(_imguiHandle);
#endif
}

void DebugWindow::beginFrame() {
#if DEBUG_WINDOW_ENABLE
	DebugGui::BeginDebugWindowGui();
#endif
}

void DebugWindow::renderFrame(CommandList* commandList) {
#if DEBUG_WINDOW_ENABLE
	InputSystem* inputSystem = InputSystem::Get();
	static bool visible = true;
	if (inputSystem->getKeyDown(InputSystem::KEY_CODE_G)) {
		visible = !visible;
	}

	DebugGui::RenderDebugWindowImgui();
	if (visible) {
		DEBUG_MARKER_CPU_GPU_SCOPED_EVENT(commandList, Color4::YELLOW, "ImGui");
		DebugGui::RenderDebugWindowGui(commandList);
	}

#endif
}

void DebugWindow::End() {
	DebugGui::End();
}
