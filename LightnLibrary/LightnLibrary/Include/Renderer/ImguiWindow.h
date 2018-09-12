#pragma once

#include <Windows.h>
#include <ThirdParty/imgui_impl_win32.h>
#include <ThirdParty/imgui_impl_dx11.h>

class ImguiWindow {

public:

	//破棄する
	static void destroy();
	
	//初期化
	static void setUpWindow(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	
	//新規レンダリングフレームを生成
	static void createNewFrame();

	//デバッグウィンドウを描画
	static void render();

	//Windowsからのイベントをデバッグ捜査に反映
	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};