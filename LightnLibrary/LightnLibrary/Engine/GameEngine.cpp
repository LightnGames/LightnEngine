#include "GameEngine.h"
#include <Renderer/GameRenderer.h>
#include <Scene/SceneManager.h>
#include <Animation/AnimationManager.h>
#include <Util/Input.h>
#include <Renderer/ImguiWindow.h>

//�֐��v���g�^�C�v�̐錾
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GameEngine::GameEngine() {
}

GameEngine::~GameEngine() {
}

void GameEngine::initialize(const HINSTANCE & hInst) {

	_gameRenderer = std::make_unique<GameRenderer>();
	_sceneManager = std::make_unique<SceneManager>();
	_animationManager = std::make_unique<AnimationManager>();

	_gameRenderer->createGameWindow(hInst, WndProc);

	_sceneManager->initialize();
}

void GameEngine::run() {

	//���b�Z�[�W���[�v
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			DispatchMessage(&msg);
		} else {

			float deltaTime = 1.0f;

			//�f�o�b�O�p�E�B���h�E�̃����_�[��������
			ImguiWindow::createNewFrame();

			//�Q�[���X�V
			_sceneManager->updateScene(deltaTime);
			_animationManager->updateAnimations(deltaTime);

			//�Q�[���`��
			_gameRenderer->draw();

			//���͂��t���[�����ƂɃ��Z�b�g
			Input::updatePerFrame();
		}
	}
}

void GameEngine::updateGame() {
	_sceneManager->updateScene(1.0f);
	_gameRenderer->draw();
}

//�E�B���h�E�v���V�[�W���\
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//�f�o�b�O�p���͂�������
	if (ImguiWindow::WndProcHandler(hWnd, iMsg, wParam, lParam)) {
		return true;
	}

	switch (iMsg) {


	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED){
			GameRenderer::instance().resize((uint32)LOWORD(lParam), (uint32)HIWORD(lParam));
		}
		return 0;

		//�j�����b�Z�[�W�i�~�{�^���Ƃ��j
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		//�L�[���͍X�V
	case WM_KEYDOWN:
		Input::updateKeyInput(true, wParam);
		break;

	case WM_KEYUP:
		Input::updateKeyInput(false, wParam);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
