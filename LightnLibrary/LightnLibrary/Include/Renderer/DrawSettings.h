#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/Camera.h>

//������J�����̏��Ȃǂ��L���b�V�����ăh���[�֐��ɔz�B
struct DrawSettings {
	ComPtr<ID3D11DeviceContext> deviceContext;
	RefPtr<Deferredbuffers> deferredBuffers;
	ComPtr<ID3D11ShaderResourceView> mainShaderResourceView;
	RefPtr<Camera> camera;
};