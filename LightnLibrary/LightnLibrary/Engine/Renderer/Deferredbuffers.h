#pragma once

#include <d3d11.h>
#include <Util/Type.h>
#include <Util/ComPtr.h>

const int BUFFER_COUNT = 3;

class Deferredbuffers {

public:

	Deferredbuffers();
	~Deferredbuffers();

	//GBuffer������
	HRESULT initialize(ComPtr<ID3D11Device>& device, uint16 width, uint16 height, float screenDepth, float screenNear);

	//GBuffer�����ׂč폜
	void cleanUp();

	//GBuffer�̃J���[�o�b�t�@�����C�������_�[�^�[�Q�b�g�ɐݒ�
	void setRenderTargets(ComPtr<ID3D11DeviceContext> context);

	//ZPrePass�p�̃����_�[�^�[�Q�b�g���Z�b�g
	void setRenderTargetEaryZ(ComPtr<ID3D11DeviceContext> context);

	//GBuffer���w��J���[�ŃN���A
	void clearRenderTargets(ComPtr<ID3D11DeviceContext> context, float red, float green, float blue, float alpha);

	//Gbuffer�̃V�F�[�_�[���\�[�X�r���[���C���f�b�N�X�Ŏ擾
	ComPtr<ID3D11ShaderResourceView> getShaderResourceView(uint16 index) const;

	ComPtr<ID3D11ShaderResourceView> getDepthStencilResource() const;

private:

	//�X�̃����_�[�^�[�Q�b�g���t�H�[�}�b�g�ƃC���f�b�N�X�Ő���
	HRESULT createRenderTarget(uint16 index, DXGI_FORMAT format, ComPtr<ID3D11Device>& device);

public:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Texture2D> _renderTargetTextureArray[BUFFER_COUNT];
	ComPtr<ID3D11RenderTargetView> _renderTargetViewArray[BUFFER_COUNT];
	ComPtr<ID3D11ShaderResourceView> _shaderResourceViewArray[BUFFER_COUNT];
	ComPtr<ID3D11Texture2D> _depthStencilBuffer;
	ComPtr<ID3D11ShaderResourceView> _depthStencilResource;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	ComPtr<ID3D11DepthStencilState> _depthStencilState;
	ComPtr<ID3D11PixelShader> _depthShader;
	D3D11_VIEWPORT _viewport;

};