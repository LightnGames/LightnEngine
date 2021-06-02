#include "ApplicationImpl.h"

ApplicationSystemImpl _applicationSystem;
InputSystemImpl _inputSystem;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	bool isDef = false;
	switch (message) {
	case WM_PAINT:
		_inputSystem.update();
		isDef = true;
		break;
	case WM_MOUSEMOVE:
		_inputSystem.setMouseEvent(Vector2(LOWORD(lParam), HIWORD(lParam)));
		isDef = true;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		isDef = true;
		break;
	}
	ApplicationImpl* app = static_cast<ApplicationImpl*>(ApplicationSystemImpl::Get()->getApplication());
	app->translateApplicationCallback(message, wParam, lParam);

	if (isDef) {
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ApplicationImpl::initialize() {
	HINSTANCE windowHandle = GetModuleHandle(NULL);

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = windowHandle;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "LightnEngine";
	RegisterClassEx(&windowClass);

	u32 width = 1920;
	u32 height = 1080;
	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	HWND hwnd = CreateWindow(
		windowClass.lpszClassName,
		windowClass.lpszClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		windowHandle,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);
	_hWnd = hwnd;
	_screenWidth = width;
	_screenHeight = height;
}

void ApplicationImpl::run() {
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void ApplicationImpl::terminate() {
}

void ApplicationImpl::translateApplicationCallback(u32 message, u64 wParam, s64 lParam) {
	u32 callbackCount = _callbacks.getCount();
	for (u32 callbackIndex = 0; callbackIndex < callbackCount; ++callbackIndex) {
		_callbacks[callbackIndex](message, wParam, lParam);
	}
}

void ApplicationImpl::registApplicationCallBack(ApplicationCallBack& callback) {
	_callbacks.push(callback);
}

ApplicationSystem* ApplicationSystem::Get(){
	return &_applicationSystem;
}

void ApplicationSystemImpl::initialize() {
}

void ApplicationSystemImpl::terminate() {
}

ApplicationSystemImpl* ApplicationSystemImpl::Get() {
	return &_applicationSystem;
}

bool InputSystemImpl::getKey(KeyCode keyCode) const {
	return isKeyDown(_keyStates[keyCode]);
}

bool InputSystemImpl::getKeyDown(KeyCode keyCode) const {
	return _keyDowns[keyCode] == 1;
}

bool InputSystemImpl::getKeyUp(KeyCode keyCode) const {
	return _keyUps[keyCode] == 1;
}

void InputSystemImpl::update() {
	u8 prevKeyState[256];
	memcpy(prevKeyState, _keyStates, LTN_COUNTOF(prevKeyState));

	GetKeyboardState(_keyStates);

	for (u16 i = 0; i < LTN_COUNTOF(prevKeyState); ++i) {
		_keyDowns[i] = (!isKeyDown(prevKeyState[i])) && isKeyDown(_keyStates[i]);
		_keyUps[i] = isKeyDown(prevKeyState[i]) && (!isKeyDown(_keyStates[i]));
	}
	
	if (isKeyDown(_keyStates[KEY_CODE_LBUTTON]) && (!isKeyDown(prevKeyState[KEY_CODE_LBUTTON]))) {
		_mousePositions[MOUSE_EVENT_L_DOWN] = _mousePosition;
	}

	if ((!isKeyDown(_keyStates[KEY_CODE_LBUTTON])) && isKeyDown(prevKeyState[KEY_CODE_LBUTTON])) {
		_mousePositions[MOUSE_EVENT_L_UP] = _mousePosition;
	}

	if (isKeyDown(_keyStates[KEY_CODE_RBUTTON]) && (!isKeyDown(prevKeyState[KEY_CODE_RBUTTON]))) {
		_mousePositions[MOUSE_EVENT_R_DOWN] = _mousePosition;
	}

	if ((!isKeyDown(_keyStates[KEY_CODE_LBUTTON])) && isKeyDown(prevKeyState[KEY_CODE_RBUTTON])) {
		_mousePositions[MOUSE_EVENT_R_UP] = _mousePosition;
	}
}

void InputSystemImpl::setMouseEvent(Vector2 position) {
	_mousePosition = position;
}

InputSystem* InputSystem::Get() {
	return &_inputSystem;
}

InputSystemImpl* InputSystemImpl::Get() {
	return &_inputSystem;
}
