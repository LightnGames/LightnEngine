#pragma once
#include <Core/CoreModuleSettings.h>
#include <Core/System.h>
#include <functional>

using ApplicationCallBack = std::function<void(u32, u64, s64)>;

struct ApplicationDesc {
};

class LTN_APP_API Application {
public:
	virtual void initialize() = 0;
	virtual void run() = 0;
	virtual void terminate() = 0;
	virtual s32* getWindowHandle() = 0;
	virtual u32 getScreenWidth() const = 0;
	virtual u32 getScreenHeight() const = 0;

	virtual void registApplicationCallBack(ApplicationCallBack& callback) = 0;
};

class LTN_APP_API ApplicationSystem {
public:
	virtual void initialize() = 0;
	virtual void terminate() = 0;
	virtual Application* getApplication() = 0;

	static ApplicationSystem* Get();
};

class LTN_APP_API InputSystem {
public:
	enum MouseEvent {
		MOUSE_EVENT_L_DOWN = 0,
		MOUSE_EVENT_L_UP,
		MOUSE_EVENT_R_DOWN,
		MOUSE_EVENT_R_UP,
		MOUSE_EVENT_COUNT
	};

	// from WinUser.h
	enum KeyCode {
		KEY_CODE_LBUTTON =       0x01,
		KEY_CODE_RBUTTON =        0x02,

		KEY_CODE_SHIFT = 0x10,
		KEY_CODE_CONTROL = 0x11,
		KEY_CODE_MENU = 0x12,
		KEY_CODE_PAUSE = 0x13,
		KEY_CODE_CAPITAL = 0x14,

		KEY_CODE_KANA = 0x15,
		KEY_CODE_HANGUL = 0x15,
		KEY_CODE_IME_ON = 0x16,
		KEY_CODE_JUNJA = 0x17,
		KEY_CODE_FINAL = 0x18,
		KEY_CODE_HANJA = 0x19,
		KEY_CODE_KANJI = 0x19,
		KEY_CODE_IME_OFF = 0x1A,

		KEY_CODE_ESCAPE = 0x1B,

		KEY_CODE_CONVERT = 0x1C,
		KEY_CODE_NONCONVERT = 0x1D,
		KEY_CODE_ACCEPT = 0x1E,
		KEY_CODE_MODECHANGE = 0x1F,

		KEY_CODE_SPACE = 0x20,
		KEY_CODE_PRIOR = 0x21,
		KEY_CODE_NEXT = 0x22,
		KEY_CODE_END = 0x23,
		KEY_CODE_HOME = 0x24,
		KEY_CODE_LEFT = 0x25,
		KEY_CODE_UP = 0x26,
		KEY_CODE_RIGHT = 0x27,
		KEY_CODE_DOWN = 0x28,
		KEY_CODE_SELECT = 0x29,
		KEY_CODE_PRINT = 0x2A,
		KEY_CODE_EXECUTE = 0x2B,
		KEY_CODE_SNAPSHOT = 0x2C,
		KEY_CODE_INSERT = 0x2D,
		KEY_CODE_DELETE = 0x2E,
		KEY_CODE_HELP = 0x2F,

		KEY_CODE_0 = 0x30,
		KEY_CODE_1 = 0x31,
		KEY_CODE_2 = 0x32,
		KEY_CODE_3 = 0x33,
		KEY_CODE_4 = 0x34,
		KEY_CODE_5 = 0x35,
		KEY_CODE_6 = 0x36,
		KEY_CODE_7 = 0x37,
		KEY_CODE_8 = 0x38,
		KEY_CODE_9 = 0x39,

		KEY_CODE_A = 0x41,
		KEY_CODE_B = 0x42,
		KEY_CODE_C = 0x43,
		KEY_CODE_D = 0x44,
		KEY_CODE_E = 0x45,
		KEY_CODE_F = 0x46,
		KEY_CODE_G = 0x47,
		KEY_CODE_H = 0x48,
		KEY_CODE_I = 0x49,
		KEY_CODE_J = 0x4A,
		KEY_CODE_K = 0x4B,
		KEY_CODE_L = 0x4C,
		KEY_CODE_N = 0x4D,
		KEY_CODE_M = 0x4E,
		KEY_CODE_O = 0x4F,
		KEY_CODE_P = 0x50,
		KEY_CODE_Q = 0x51,
		KEY_CODE_R = 0x52,
		KEY_CODE_S = 0x53,
		KEY_CODE_T = 0x54,
		KEY_CODE_U = 0x55,
		KEY_CODE_V = 0x56,
		KEY_CODE_W = 0x57,
		KEY_CODE_X = 0x58,
		KEY_CODE_Y = 0x59,
		KEY_CODE_Z = 0x5A,
	};

	enum KeyState {
		KEY_STATE_DOWN = 0,
		KEY_STATE_UP,
	};

	virtual bool getKey(KeyCode keyCode) const = 0;
	virtual bool getKeyDown(KeyCode keyCode) const = 0;
	virtual bool getKeyUp(KeyCode keyCode) const = 0;
	virtual Vector2 getMousePosition() const = 0;
	virtual Vector2 getMousePosition(MouseEvent mouseEvent) const = 0;

	static InputSystem* Get();
};