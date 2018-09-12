#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <LMath.h>
#include <memory>

class RenderTarget;
const int BUFFER_COUNT = 4;

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

	//���C�e�B���O���ʗp�̃����_�[�^�[�Q�b�g�ɐ؂�ւ�
	void setRenderTargetLighting(ComPtr<ID3D11DeviceContext> deviceContext);

	//ZPrePass�p�̃����_�[�^�[�Q�b�g���Z�b�g
	void setRenderTargetEaryZ(ComPtr<ID3D11DeviceContext> context);

	//GBuffer���w��J���[�ŃN���A
	void clearRenderTargets(ComPtr<ID3D11DeviceContext> context, float red, float green, float blue, float alpha);

	void setViewPort(ComPtr<ID3D11DeviceContext> deviceContext);

	ID3D11ShaderResourceView* getShaderResourceView(uint16 index) const;

	RefPtr<RenderTarget> getRenderTarget(uint16 index);

	ComPtr<ID3D11ShaderResourceView> getDepthStencilResource() const;

	ID3D11DepthStencilView* getDepthStencilView(bool readOnly);

	Vector2 getGBufferSize() const;

private:

	//�X�̃����_�[�^�[�Q�b�g���t�H�[�}�b�g�ƃC���f�b�N�X�Ő���
	HRESULT createRenderTarget(uint16 index, DXGI_FORMAT format, ComPtr<ID3D11Device>& device);

private:

	uint16 _width;
	uint16 _height;

	ComPtr<ID3D11Texture2D> _depthStencilBuffer;
	ComPtr<ID3D11ShaderResourceView> _depthStencilSRV;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	ComPtr<ID3D11DepthStencilView> _depthStencilViewReadOnly;
	ComPtr<ID3D11DepthStencilState> _depthStencilState;
	std::unique_ptr<RenderTarget> _renderTargets[BUFFER_COUNT];
	D3D11_VIEWPORT _viewport;

};