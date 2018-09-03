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
#include "Mesh/SkyBox.h"
#include "StaticInstancedMeshRenderer.h"
#include <ThirdParty/ImGui/imgui.h>
#include "TileBasedLightCulling.h"
#include <Renderer/Light/LightTypes.h>
#include <Renderer/GraphicsResourceManager.h>
#include <Renderer/RendererUtil.h>
#include <Components/CameraComponent.h>
#include "PostEffect/PostProcess.h"
#include "GraphicsBuffers.h"
#include "PostEffect/SSAO.h"

std::unique_ptr<DrawSettings> drawSettings;
template<> GameRenderer* Singleton<GameRenderer>::mSingleton = 0;

GameRenderer::GameRenderer() :_width{ 1280 }, _height{ 720 },
_device{ nullptr },
_deviceContext{ nullptr },
_swapChain{ nullptr },
_sceneRendererManager{ nullptr },
_deferredBuffers{ nullptr },
_orthoScreen{ nullptr },
_tileCulling{ nullptr },
_graphicsResourceManager{ nullptr },
_postProcess{ nullptr },
_ssao{ nullptr } {

	_sceneRendererManager = std::make_unique<SceneRendererManager>();
	drawSettings = std::make_unique<DrawSettings>();
	_deferredBuffers = std::make_unique<Deferredbuffers>();
	_orthoScreen = std::make_unique<OrthoScreen>();
	_debugGeometryRenderer = std::make_unique<DebugGeomtryRenderer>();
	_staticInstancedMeshRenderer = std::make_unique<StaticInstancedMeshRenderer>();
	_tileCulling = std::make_unique<TileBasedLightCulling>();
	_graphicsResourceManager = std::make_unique<GraphicsResourceManager>();
	_postProcess = std::make_unique<PostProcess>();
	_ssao = std::make_unique<SSAO>();
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

struct WorldConstant {
	Vector4 time;
};
ComPtr<ID3D11Buffer> worldConstant;
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
	//D3D11_CREATE_DEVICE_DEBUG
	HRESULT hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &featureLevels
		, 1, D3D11_SDK_VERSION, &sd, _swapChain.ReleaseAndGetAddressOf(), _device.ReleaseAndGetAddressOf(), nullptr, _deviceContext.ReleaseAndGetAddressOf());

	if (FAILED(hr)) {
		return hr;
	}

	drawSettings->deviceContext = _deviceContext;


	//デバッグ用ウィンドウを初期化
	ImguiWindow::setUpWindow(_hWnd, _device.Get(), _deviceContext.Get());

	_debugGeometryRenderer->initialize(_device);
	_graphicsResourceManager->initialize(_device);
	_postProcess->initialize(_device);
	_ssao->initialize(_device);

	_sceneRendererManager->setUp();
	RendererUtil::createConstantBuffer(worldConstant, sizeof(WorldConstant), _device);

	return S_OK;
}

void GameRenderer::draw() {

	//画面クリア
	const float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };
	_orthoScreen->clearMainRenderTarget(clearColor);

	//カメラ情報をセットアップ
	auto& mainCamera = CameraComponent::mainCamera;
	drawSettings->camera = mainCamera->camera();
	drawSettings->camera->mtxView = mainCamera->cameraMatrix().inverse();
	drawSettings->camera->position = mainCamera->getWorldPosition();
	drawSettings->camera->rotation = mainCamera->getWorldRotation();

	//StaticInstanceMeshのカリング情報をクリア
	_staticInstancedMeshRenderer->clearCullingBuffer(_deviceContext, mainCamera->refCamera());

	//ZPrePass
	_deferredBuffers->setRenderTargetEaryZ(_deviceContext);

	static float time = 0.0f;
	time += 0.0166666666f;
	WorldConstant world;
	world.time.x = time;
	world.time.y = std::pow(time, 2);

	_deviceContext->UpdateSubresource(worldConstant.Get(), 0, 0, &world, 0, 0);
	_deviceContext->VSSetConstantBuffers(2, 1, worldConstant.GetAddressOf());

	for (auto&& sm : _sceneRendererManager->renderableEntities()) {
		sm->drawDepth(*drawSettings.get());
	}

	//GBufferに描画するために切り替え・クリア
	_deferredBuffers->setRenderTargets(_deviceContext);
	_deferredBuffers->clearRenderTargets(_deviceContext, 0.0f, 0.0f, 0.0f, 0.0f);

	//Gbufferに描画
	for (auto&& sm : _sceneRendererManager->renderableEntities()) {
		sm->draw(*drawSettings.get());
	}

	//SkyBoxステンシル描画
	_sceneRendererManager->skyBox()->drawStencil(*drawSettings);

	//レンダーターゲット切り替え・ライティング設定 ここから読み取り専用デプスステンシル
	_deviceContext->OMSetRenderTargets(1, _deferredBuffers->getRenderTarget(3)->ppRtv(), _deferredBuffers->getDepthStencilView(true));
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_deviceContext->OMSetBlendState(_orthoScreen->_blendState.Get(), blendFactor, D3D11_DEFAULT_SAMPLE_MASK);

	//Skybox描画
	_sceneRendererManager->skyBox()->draw(*drawSettings);

	//ライティング用ステンシルモード切替
	_orthoScreen->setStencilStateLight();

	//スクリーン描画用頂点バッファをセット
	_orthoScreen->setOrthoScreenVertex();

	//ライト描画
	for (auto&& l : _sceneRendererManager->lightEntities()) {
		l->draw(*drawSettings.get());
	}

	//SSAO
	_ssao->draw(_deviceContext, _deferredBuffers.get(), _orthoScreen.get(), drawSettings->camera);

	//TileBasedLighting
	const Matrix4 mtxCamera = mainCamera->cameraMatrix().inverse();
	const TileBasedPointLightType* pointLights = _sceneRendererManager->pointLights(mtxCamera);
	const TileBasedSpotLightType* spotLights = _sceneRendererManager->spotLights(mtxCamera);
	_tileCulling->draw(*drawSettings, pointLights, spotLights);

	//ポストプロセス
	_deviceContext->PSSetShaderResources(7, 1, _ssao->ssaoResource());
	_postProcess->draw(_deviceContext, _deferredBuffers.get(), _orthoScreen.get(), drawSettings->camera);

	//デバッグジオメトリを描画
	const auto& lines = _sceneRendererManager->debugLines();
	const auto& boxs = _sceneRendererManager->debugBoxs();
	const auto& spheres = _sceneRendererManager->debugSpheres();
	_debugGeometryRenderer->draw(spheres, boxs, lines, *drawSettings);

	ImGui::Begin("Winds");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

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

void GameRenderer::setOrthoScreenVertex() {
	_orthoScreen->setOrthoScreenVertex();
}

void GameRenderer::cleanUpRenderTargets() {

	_orthoScreen->cleanUp();
	_deferredBuffers->cleanUp();
}

void GameRenderer::createRenderTargets() {

	HRESULT hr;

	hr = _orthoScreen->initialize(_device, _deviceContext, _swapChain, _width, _height);
	hr = _deferredBuffers->initialize(_device, _width, _height, 0.1f, 1000);
	hr = _tileCulling->initialize(_device, _width, _height);
	hr = _postProcess->setupRenderResource(_device, _width, _height);
	hr = _ssao->setupRenderResource(_device, _width, _height);

	drawSettings->mainShaderResourceView = _orthoScreen->getShaderResourceView();
	drawSettings->deferredBuffers = _deferredBuffers.get();
}
