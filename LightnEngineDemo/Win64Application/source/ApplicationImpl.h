#pragma once
#include <Core/Application.h>
#include <Windows.h>

class ApplicationImpl :public Application {
public:
	static constexpr u32 APPLICATION_CALLBACK_COUNT_MAX = 32;

	void initialize() override;
	void run() override;
	void terminate() override;
	virtual s32* getWindowHandle() override {
		return reinterpret_cast<s32*>(_hWnd);
	}
	virtual u32 getScreenWidth() const override {
		return _screenWidth;
	}
	virtual u32 getScreenHeight() const override {
		return _screenHeight;
	}

	void translateApplicationCallback(u32 message, u64 wParam, s64 lParam);
	void registApplicationCallBack(ApplicationCallBack& callback) override;

private:
	HWND _hWnd = nullptr;
	u32 _screenWidth = 0;
	u32 _screenHeight = 0;
	LinerArray<ApplicationCallBack, APPLICATION_CALLBACK_COUNT_MAX> _callbacks;
};

class ApplicationSystemImpl :public ApplicationSystem {
public:
	virtual void initialize() override;
	virtual void terminate() override;
	virtual Application* getApplication() override { return &_application; };

	static ApplicationSystemImpl* Get();

private:
	ApplicationImpl _application;
};

class InputSystemImpl :public InputSystem {
public:
	virtual bool getKey(KeyCode keyCode) const override;
	virtual bool getKeyDown(KeyCode keyCode) const override;
	virtual bool getKeyUp(KeyCode keyCode) const override;
	virtual Vector2 getMousePosition() const override { return _mousePosition; }
	virtual Vector2 getMousePosition(MouseEvent mouseEvent) const override { return _mousePositions[mouseEvent]; }

	void update();
	void setMouseEvent(Vector2 position);

	static InputSystemImpl* Get();
private:
	bool isKeyDown(u8 key) const { return key & 0x80; }

private:
	u8 _keyStates[256];
	u8 _keyDowns[256];
	u8 _keyUps[256];
	Vector2 _mousePosition;
	Vector2 _mousePositions[MOUSE_EVENT_COUNT] = {};
};