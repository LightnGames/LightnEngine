#pragma once

#include <Windows.h>
#include <ThirdParty/imgui_impl_win32.h>
#include <ThirdParty/imgui_impl_dx11.h>

class ImguiWindow {

public:

	//�j������
	static void destroy();
	
	//������
	static void setUpWindow(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	
	//�V�K�����_�����O�t���[���𐶐�
	static void createNewFrame();

	//�f�o�b�O�E�B���h�E��`��
	static void render();

	//Windows����̃C�x���g���f�o�b�O�{���ɔ��f
	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};