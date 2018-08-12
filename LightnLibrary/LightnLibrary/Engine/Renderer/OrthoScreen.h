#pragma once

#include <d3d11.h>
#include <string>

#include <Util/Singleton.h>
#include <Util/RefPtr.h>
#include <Util/ComPtr.h>
#include <LMath.h>
#include <Util/Type.h>

class OrthoScreen {

public:
	OrthoScreen();
	~OrthoScreen();

	HRESULT initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& deviceContext, ComPtr<IDXGISwapChain>& swapChain, uint16 width, uint16 height);

	//�����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�ɐ؂�ւ�
	void setBackBuffer();

	//�o�b�N�o�b�t�@�ƃX�e���V���r���[���Z�b�g
	void setBackBufferAndDSV(ComPtr<ID3D11DepthStencilView> dsv);

	//���C�e�B���O�p�X�e���V���X�e�[�g���Z�b�g
	void setStencilStateLight();

	//���C�������_�[�^�[�Q�b�g���N���A
	void clearMainRenderTarget(const float color[4]);

	//�N���[���A�b�v
	void cleanUp();

	//�X�N���[���`��p���_�o�b�t�@�ɐ؂�ւ�
	void setOrthoScreenVertex();

	//���C�������_�[�^�[�Q�b�g�̃V�F�[�_�[���\�[�X�r���[���擾
	ComPtr<ID3D11ShaderResourceView> getShaderResourceView() const;

private:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;

	ComPtr<ID3D11DepthStencilState> _stencilState;
	ComPtr<ID3D11Buffer> _screenVertexBuffer;
	ComPtr<ID3D11BlendState> _blendState;
	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
	D3D11_VIEWPORT _viewPort;
};