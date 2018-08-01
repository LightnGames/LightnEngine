#pragma once

#include <d3d11.h>
#include <Util/ComPtr.h>
#include <Util/RefPtr.h>
#include "Deferredbuffers.h"

//������J�����̏��Ȃǂ��L���b�V�����ăh���[�֐��ɔz�B
struct DrawSettings {
	ComPtr<ID3D11DeviceContext> deviceContext;
	RefPtr<Deferredbuffers> deferredBuffers;
	ComPtr<ID3D11ShaderResourceView> mainShaderResourceView;
};