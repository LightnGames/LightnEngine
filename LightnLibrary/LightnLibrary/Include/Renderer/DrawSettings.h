#pragma once

#include <d3d11.h>
#include <Util/Util.h>
#include <Renderer/Deferredbuffers.h>
#include <Renderer/Camera.h>

//いずれカメラの情報などをキャッシュしてドロー関数に配達
struct DrawSettings {
	ComPtr<ID3D11DeviceContext> deviceContext;
	RefPtr<Deferredbuffers> deferredBuffers;
	ComPtr<ID3D11ShaderResourceView> mainShaderResourceView;
	RefPtr<Camera> camera;
};