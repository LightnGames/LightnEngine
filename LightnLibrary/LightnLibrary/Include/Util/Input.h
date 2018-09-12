#pragma once

#include <Windows.h>
#include <list>
#include <vector>
#include <utility>

using KeyCodeVector = std::vector<UCHAR>;

enum class KeyCode{
	Clear = 0x0c,
	Return = 0x0d,
	Shift = 0x10,
	Control = 0x11,
	Menu = 0x12,
	Pause = 0x13,
	Capital = 0x14,
	Escape = 0x1b,
	Convert = 0x1c,
	Nonconvert = 0x1d,
	Accept = 0x1e,
	ModeChange = 0x1f,
	Space = 0x20,
	Prior = 0x21,
	Next = 0x22,
	End = 0x23,
	Home = 0x24,
	Left = 0x25,
	Up = 0x26,
	Right = 0x27,
	Down = 0x28,
	Select = 0x29,
	Print = 0x2a,
	Execute = 0x2b,
	Snapshot = 0x2c,
	Insert = 0x2d,
	Delete = 0x2e,
	Help = 0x2f,
	Lwin = 0x5b,
	Rwin = 0x5c,
	Apps = 0x5d,
	Sleep = 0x5f,
	Numpad0 = 0x60,
	Numpad1 = 0x61,
	Numpad2 = 0x62,
	Numpad3 = 0x63,
	Numpad4 = 0x64,
	Numpad5 = 0x65,
	Numpad6 = 0x66,
	Numpad7 = 0x67,
	Numpad8 = 0x68,
	Numpad9 = 0x69,
	Multiply = 0x6a,
	Add = 0x6b,
	Separator = 0x6c,
	Subtract = 0x6d,
	Decimal = 0x6e,
	Divide = 0x6f,
	F1 = 0x70,
	F2 = 0x71,
	F3 = 0x72,
	F4 = 0x73,
	F5 = 0x74,
	F6 = 0x75,
	F7 = 0x76,
	F8 = 0x77,
	F9 = 0x78,
	F10 = 0x79,
	F11 = 0x7a,
	F12 = 0x7b,
	Numlock = 0x90,
	Scroll = 0x91,
	A = 0x41,
	B = 0x42,
	C = 0x43,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	G = 0x47,
	H = 0x48,
	I = 0x49,
	J = 0x4A,
	K = 0x4B,
	L = 0x4C,
	N = 0x4D,
	M = 0x4E,
	O = 0x4F,
	P = 0x50,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	T = 0x54,
	U = 0x55,
	V = 0x56,
	W = 0x57,
	X = 0x58,
	Y = 0x59,
	Z = 0x5A,
};

class Input{

public:

	//キー状態を更新する
	static void updateKeyInput(bool pressed, WPARAM wParam);

	//フレーム更新ごとの処理
	static void updatePerFrame();

	//キーが押されているか
	static bool getKey(const KeyCode code);

	//キーが押されたフレームか
	static bool getKeyDown(const KeyCode code);

	//キーが離されたフレームか
	static bool getKeyUp(const KeyCode code);

private:

	static bool containtsKey(KeyCodeVector& vector,const KeyCode code);

private:

	static KeyCodeVector _pressKeys;
	static KeyCodeVector _beforePressKeys;
};

