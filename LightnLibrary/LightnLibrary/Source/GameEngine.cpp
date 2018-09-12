#include "GameEngine.h"
#include <Renderer/GameRenderer.h>
#include <Scene/SceneManager.h>
#include <Animation/AnimationManager.h>
#include <Util/Input.h>
#include <Renderer/ImguiWindow.h>

//関数プロトタイプの宣言
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

	//メッセージループ
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			DispatchMessage(&msg);
		} else {

			float deltaTime = 1.0f;

			//デバッグ用ウィンドウのレンダーを初期化
			ImguiWindow::createNewFrame();

			//ゲーム更新
			_sceneManager->updateScene(deltaTime);
			_animationManager->updateAnimations(deltaTime);

			//ゲーム描画
			_gameRenderer->draw();

			//入力をフレームごとにリセット
			Input::updatePerFrame();
		}
	}
}

void GameEngine::updateGame() {
	_sceneManager->updateScene(1.0f);
	_gameRenderer->draw();
}

//ウィンドウプロシージャ―
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//デバッグ用入力だったら
	if (ImguiWindow::WndProcHandler(hWnd, iMsg, wParam, lParam)) {
		return true;
	}

	switch (iMsg) {


	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED){
			GameRenderer::instance().resize((uint32)LOWORD(lParam), (uint32)HIWORD(lParam));
		}
		return 0;

		//破棄メッセージ（×ボタンとか）
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		//キー入力更新
	case WM_KEYDOWN:
		Input::updateKeyInput(true, wParam);
		break;

	case WM_KEYUP:
		Input::updateKeyInput(false, wParam);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
