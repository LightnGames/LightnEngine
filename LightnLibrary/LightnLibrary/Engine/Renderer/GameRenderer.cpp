#include "GameRenderer.h"
#include "SceneRendererManager.h"

#include "DrawSettings.h"
#include "RenderableEntity.h"
#include "Deferredbuffers.h"
#include "OrthoScreen.h"
#include <LMath.h>
#include "ImguiWindow.h"
#include "LightEntity.h"
#include "Mesh/DebugGeometry.h"
#include "StaticInstancedMeshRenderer.h"
#include <ThirdParty/ImGui/imgui.h>

std::unique_ptr<DrawSettings> drawSettings;
template<> GameRenderer* Singleton<GameRenderer>::mSingleton = 0;

GameRenderer::GameRenderer() :_width{ 1280 }, _height{ 720 },
_device{ nullptr },
_deviceContext{ nullptr },
_swapChain{ nullptr },
_sceneRendererManager{ nullptr },
_deferredBuffers{ nullptr },
_orthoScreen{ nullptr } {

	_sceneRendererManager = std::make_unique<SceneRendererManager>();
	drawSettings = std::make_unique<DrawSettings>();
	_deferredBuffers = std::make_unique<Deferredbuffers>();
	_orthoScreen = std::make_unique<OrthoScreen>();
	_debugGeometryRenderer = std::make_unique<DebugGeomtryRenderer>();
	_staticInstancedMeshRenderer = std::make_unique<StaticInstancedMeshRenderer>();

}

GameRenderer::~GameRenderer() {
	ImguiWindow::destroy();
}

HRESULT GameRenderer::createGameWindow(const HINSTANCE & hInst, WNDPROC lpfnWndProc) {

	//ウィンドウ拡張クラスを生成
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = lpfnWndProc;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc.lpszClassName = _windowName.c_str();

	//生成できていなければエラー
	if (!RegisterClassEx(&wc)) {
		return S_FALSE;
	}

	//生成したウィンドウを登録&表示
	_hWnd = CreateWindow(_windowName.c_str(), _windowName.c_str(), WS_OVERLAPPEDWINDOW, 0, 0, _width, _height, 0, 0, hInst, 0);

	//DirectX初期化
	initDirect3D();

	//ウィンドウ表示
	ShowWindow(_hWnd, SW_SHOW);

	return S_OK;
}

HRESULT GameRenderer::initDirect3D() {

	//スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 2;
	sd.BufferDesc.Width = _width;
	sd.BufferDesc.Height = _height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	//使用ドライバータイプを指定
	D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;

	//スワップチェイン初期化
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevels
		, 1, D3D11_SDK_VERSION, &sd, _swapChain.ReleaseAndGetAddressOf(), _device.ReleaseAndGetAddressOf(), nullptr, _deviceContext.ReleaseAndGetAddressOf());

	if (FAILED(hr)) {
		return hr;
	}

	drawSettings->deviceContext = _deviceContext;


	//デバッグ用ウィンドウを初期化
	ImguiWindow::setUpWindow(_hWnd, _device.Get(), _deviceContext.Get());

	_debugGeometryRenderer->initialize(_device);

	return S_OK;
}


void GameRenderer::draw() {

	//画面クリア
	const float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
	_orthoScreen->clearMainRenderTarget(clearColor);

	//StaticInstanceMeshのカリング情報をクリア
	_staticInstancedMeshRenderer->clearCullingBuffer(_deviceContext);

	//ZPrePass
	_deferredBuffers->setRenderTargetEaryZ(_deviceContext);

	for (auto&& sm : _sceneRendererManager->renderableEntities()) {
		sm->drawDepth(*drawSettings.get());
	}

	//GBufferに描画するために切り替え・クリア
	_deferredBuffers->setRenderTargets(_deviceContext);
	_deferredBuffers->clearRenderTargets(_deviceContext, 0.0f, 0.0f, 0.0f, 0.0f);

	ImGui::Begin("Winds");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	//Gbufferに描画
	for (auto&& sm : _sceneRendererManager->renderableEntities()) {
		sm->draw(*drawSettings.get());
	}

	//レンダーターゲット切り替え
	_orthoScreen->setBackBuffer();

	//スクリーン描画用頂点バッファをセット
	_orthoScreen->setOrthoScreenVertex();

	//ライト描画
	for (auto&& l : _sceneRendererManager->lightEntities()) {
		l->draw(*drawSettings.get());
	}

	//デバッグジオメトリを描画
	const auto& lines = _sceneRendererManager->debugLines();
	const auto& boxs = _sceneRendererManager->debugBoxs();
	const auto& spheres = _sceneRendererManager->debugSpheres();
	_debugGeometryRenderer->draw(spheres, boxs, lines, *drawSettings);

	//デバッグウィンドウを描画
	ImguiWindow::render();

	//画面更新
	_swapChain->Present(1, 0);

	_sceneRendererManager->clearDebugGeometry();
}

void GameRenderer::resize(uint16 width, uint16 height) {

	_width = width;
	_height = height;

	ImGui_ImplDX11_InvalidateDeviceObjects();
	cleanUpRenderTargets();
	_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	createRenderTargets();
	ImGui_ImplDX11_CreateDeviceObjects();
}

const Vector2 GameRenderer::screenSize() const {
	return Vector2(_width, _height);
}

RefPtr<SceneRendererManager> GameRenderer::sceneRendererManager() {
	return _sceneRendererManager.get();
}

const ComPtr<ID3D11Device>& GameRenderer::device() const {
	return _device;
}

const ComPtr<ID3D11DeviceContext>& GameRenderer::deviceContext() const {
	return _deviceContext;
}

void GameRenderer::cleanUpRenderTargets() {

	_orthoScreen->cleanUp();
	_deferredBuffers->cleanUp();
}

void GameRenderer::createRenderTargets() {

	HRESULT hr;

	hr = _orthoScreen->initialize(_device, _deviceContext, _swapChain, _width, _height);
	hr = _deferredBuffers->initialize(_device, _width, _height, 0.1f, 1000);

	drawSettings->mainShaderResourceView = _orthoScreen->getShaderResourceView();
	drawSettings->deferredBuffers = _deferredBuffers.get();

}
