#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>

#include <Util/Singleton.h>
#include <Util/Util.h>
#include <LMath.h>

class SceneRendererManager;
class Deferredbuffers;
class OrthoScreen;
class DebugGeomtryRenderer;
class StaticInstancedMeshRenderer;
class TileBasedLightCulling;
class GraphicsResourceManager;
class PostProcess;
class SSAO;

class GameRenderer :public Singleton<GameRenderer>{

public:

	GameRenderer();

	~GameRenderer();

	//�Q�[���E�B���h�E����
	HRESULT createGameWindow(const HINSTANCE & hInst, WNDPROC lpfnWndProc);

	//DirectX������
	HRESULT initDirect3D();

	//�Q�[����`��
	void draw();

	//��ʂ����T�C�Y
	void resize(uint16 width, uint16 height);

	//��ʃT�C�Y���擾
	const Vector2 screenSize() const;

	RefPtr<SceneRendererManager> sceneRendererManager();

	GameRenderer(const GameRenderer& other) = delete;

	const ComPtr<ID3D11Device>& device() const;
	const ComPtr<ID3D11DeviceContext>& deviceContext() const;

	void setOrthoScreenVertex();

private:

	//���C�������_�[�^�[�Q�b�g���폜
	void cleanUpRenderTargets();

	//���C�������_�[�^�[�Q�b�g�𐶐�
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
	std::unique_ptr<TileBasedLightCulling> _tileCulling;
	std::unique_ptr<GraphicsResourceManager> _graphicsResourceManager;
	std::unique_ptr<PostProcess> _postProcess;
	std::unique_ptr<SSAO> _ssao;
};