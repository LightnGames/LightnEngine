#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>

#include <Util/Singleton.h>
#include <Util/RefPtr.h>
#include <Util/ComPtr.h>
#include <LMath.h>
#include <Util/Type.h>

class SceneRendererManager;
class Deferredbuffers;
class OrthoScreen;
class DebugGeomtryRenderer;
class StaticInstancedMeshRenderer;

class GameRenderer :public Singleton<GameRenderer>{

public:

	GameRenderer();

	~GameRenderer();

	//ゲームウィンドウ生成
	HRESULT createGameWindow(const HINSTANCE & hInst, WNDPROC lpfnWndProc);

	//DirectX初期化
	HRESULT initDirect3D();

	//ゲームを描画
	void draw();

	//画面をリサイズ
	void resize(uint16 width, uint16 height);

	//画面サイズを取得
	const Vector2 screenSize() const;

	RefPtr<SceneRendererManager> sceneRendererManager();

	GameRenderer(const GameRenderer& other) = delete;

	const ComPtr<ID3D11Device>& device() const;
	const ComPtr<ID3D11DeviceContext>& deviceContext() const;

private:

	//メインレンダーターゲットを削除
	void cleanUpRenderTargets();

	//メインレンダーターゲットを生成
	void createRenderTargets();

private:

	std::string _windowName{ "LightnTest" };
	HWND _hWnd;
	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;
	ComPtr<IDXGISwapChain> _swapChain;

	std::unique_ptr<SceneRendererManager> _sceneRendererManager;
	std::unique_ptr<Deferredbuffers> _deferredBuffers;
	std::unique_ptr<OrthoScreen> _orthoScreen;
	std::unique_ptr<DebugGeomtryRenderer> _debugGeometryRenderer;
	std::unique_ptr<StaticInstancedMeshRenderer> _staticInstancedMeshRenderer;
};