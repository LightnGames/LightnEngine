#include <Renderer/StaticInstancedMeshRenderer.h>
#include <Collider/AABB.h>
#include <vector>
#include <algorithm>
#include <Renderer/RendererUtil.h>
#include <Renderer/Mesh/DebugGeometry.h>
#include <Renderer/SceneRendererManager.h>
#include <Renderer/Camera.h>

template<> StaticInstancedMeshRenderer* Singleton<StaticInstancedMeshRenderer>::mSingleton = 0;

StaticInstancedMeshRenderer::StaticInstancedMeshRenderer() {
}


HRESULT StaticInstancedMeshRenderer::initialize(ComPtr<ID3D11Device> device, uint32 maxDrawCount, const std::vector<uint32>& indexList) {

	_data = std::make_unique<StaticInstanceMeshData>();

	HRESULT hr;

	//インスタンスMatrixバッファ
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = sizeof(Matrix4)*maxDrawCount;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(Matrix4);

	hr = device->CreateBuffer(&bufferDesc, nullptr, _data->instanceMatrixBuffer.ReleaseAndGetAddressOf());

	//インスタンスMatrixアンオーダードアクセスビュー
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.NumElements = maxDrawCount;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

	hr = device->CreateUnorderedAccessView(_data->instanceMatrixBuffer.Get(), &uavDesc, _data->instanceMatrixUAV.ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(_data->instanceMatrixBuffer.Get(), 0, _data->instanceMatrixSRV.ReleaseAndGetAddressOf());
	
	_meshTypeCount = static_cast<uint32>(indexList.size());

	uint32 drawListLength = indexList.size() * 5;
	std::vector<uint32> drawListInfo;
	drawListInfo.reserve(drawListLength);

	for (auto&& m : indexList) {
		drawListInfo.emplace_back(m);
		drawListInfo.emplace_back(0);
		drawListInfo.emplace_back(0);
		drawListInfo.emplace_back(0);
		drawListInfo.emplace_back(0);
	}

	//インスタンスドロー用のバッファ Indirectフラグも立てる
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32) * drawListLength);
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	D3D11_SUBRESOURCE_DATA initDataI;
	initDataI.pSysMem = drawListInfo.data();

	hr = device->CreateBuffer(&bufferDesc, &initDataI, _data->indtsnceDrawListBuffer.ReleaseAndGetAddressOf());

	//インスタンスドロー用アンオーダードアクセスビュー
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.Buffer.NumElements = static_cast<UINT>(drawListLength);
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

	hr = device->CreateUnorderedAccessView(_data->indtsnceDrawListBuffer.Get(), &uavDesc, _data->instanceDrawListUAV.ReleaseAndGetAddressOf());


	RendererUtil::createComputeShader("GPUCulling.cso", _data->cullingShader, device);
	RendererUtil::createComputeShader("CreateGpuInstanceList.cso", _data->clearInstanceListShader, device);
	RendererUtil::createConstantBuffer(_data->frustumCullingBuffer, sizeof(FrustumCullingInfo), device);

	return S_OK;
}

void StaticInstancedMeshRenderer::clearCullingBuffer(ComPtr<ID3D11DeviceContext> deviceContext, const Camera& camera) {
	const uint32 resetCounter = 0;

	FrustumCullingInfo info;
	calculateFrustumPlanes(info.planes, camera);

	deviceContext->UpdateSubresource(_data->frustumCullingBuffer.Get(), 0, 0, &info, 0, 0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, _data->instanceMatrixUAV.GetAddressOf(), &resetCounter);
	deviceContext->CSSetUnorderedAccessViews(1, 1, _data->instanceDrawListUAV.GetAddressOf(), 0);

	deviceContext->CSSetShader(_data->clearInstanceListShader.Get(), 0, 0);
	deviceContext->Dispatch(_meshTypeCount, 1, 1);

	ID3D11Buffer* ppCBNULL[1] = { 0 };
	ID3D11ShaderResourceView* ppSRVNULL[2] = { 0, 0 };
	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { 0 };

	deviceContext->CSSetShader(0, 0, 0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, 0);
	deviceContext->CSSetShaderResources(0, 2, ppSRVNULL);
	deviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);
}

void StaticInstancedMeshRenderer::clearCullingBufferShadow(ComPtr<ID3D11DeviceContext> deviceContext, const Camera & camera) {

	const uint32 resetCounter = 0;

	FrustumCullingInfo info;
	info.planes[0].normal = Quaternion::rotVector(camera.rotation, Vector3::right);
	info.planes[1].normal = -Quaternion::rotVector(camera.rotation, Vector3::right);
	info.planes[2].normal = Quaternion::rotVector(camera.rotation, Vector3::up);
	info.planes[3].normal = -Quaternion::rotVector(camera.rotation, Vector3::up);
	info.planes[4].normal = Quaternion::rotVector(camera.rotation, Vector3::forward);
	info.planes[5].normal = -Quaternion::rotVector(camera.rotation, Vector3::forward);

	info.planes[0].position = Quaternion::rotVector(camera.rotation, Vector3(-2.0f / camera.mtxProj.m[0][0], 0, 0)) +camera.position;
	info.planes[1].position = Quaternion::rotVector(camera.rotation, Vector3( 2.0f / camera.mtxProj.m[0][0], 0, 0)) + camera.position;
	info.planes[2].position = Quaternion::rotVector(camera.rotation, Vector3(0, -2.0f / camera.mtxProj.m[1][1], 0)) + camera.position;
	info.planes[3].position = Quaternion::rotVector(camera.rotation, Vector3(0, 2.0f / camera.mtxProj.m[1][1], 0)) + camera.position;

	deviceContext->UpdateSubresource(_data->frustumCullingBuffer.Get(), 0, 0, &info, 0, 0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, _data->instanceMatrixUAV.GetAddressOf(), &resetCounter);
	deviceContext->CSSetUnorderedAccessViews(1, 1, _data->instanceDrawListUAV.GetAddressOf(), 0);

	deviceContext->CSSetShader(_data->clearInstanceListShader.Get(), 0, 0);
	deviceContext->Dispatch(_meshTypeCount, 1, 1);

	ID3D11Buffer* ppCBNULL[1] = { 0 };
	ID3D11ShaderResourceView* ppSRVNULL[2] = { 0, 0 };
	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { 0 };

	deviceContext->CSSetShader(0, 0, 0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, 0);
	deviceContext->CSSetShaderResources(0, 2, ppSRVNULL);
	deviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);

}

void StaticInstancedMeshRenderer::calculateFrustumPlanes(Plane planes[6], const Camera& camera) {

	const Quaternion& viewRotate = camera.rotation;
	const Vector3& viewPosition = camera.position;
	const Matrix4& mtxProj = camera.mtxProj;
	const float& farClip = camera.farClip;
	const float& nearClip = camera.nearClip;
	const Vector3 forward = Quaternion::rotVector(viewRotate, Vector3::forward);

	// 0: Left, 1: Right, 2: Bottm, 3: Top
	const float fov = radianFromDegree(camera.fov);
	const float sin = std::sin(fov);
	const float cos = std::cos(fov);
	planes[0].normal = Quaternion::rotVector(viewRotate, Vector3( cos,0, sin));
	planes[1].normal = Quaternion::rotVector(viewRotate, Vector3(-cos,0, sin));
	planes[2].normal = Quaternion::rotVector(viewRotate, Vector3(0, sin, cos));
	planes[3].normal = Quaternion::rotVector(viewRotate, Vector3(0,-sin, cos));

	planes[0].position = viewPosition;
	planes[1].position = viewPosition;
	planes[2].position = viewPosition;
	planes[3].position = viewPosition;

	// for the near plane
	float a = mtxProj.m[0][3] + mtxProj.m[0][2];
	float b = mtxProj.m[1][3] + mtxProj.m[1][2];
	float c = mtxProj.m[2][3] + mtxProj.m[2][2];

	Vector3 normal = Vector3(a, b, c).normalize();
	normal = Quaternion::rotVector(viewRotate, normal);

	Plane result;
	result.normal = normal;
	result.position = viewPosition + (forward * nearClip);

	planes[4] = result;

	// for the far plane
	a = mtxProj.m[0][3] - mtxProj.m[0][2];
	b = mtxProj.m[1][3] - mtxProj.m[1][2];
	c = mtxProj.m[2][3] - mtxProj.m[2][2];

	normal = Vector3(a, b, c).normalize();
	normal = Quaternion::rotVector(viewRotate, normal);

	result.normal = normal;
	result.position = viewPosition + (forward * nearClip) + (forward * farClip);

	planes[5] = result;
}

RefPtr<StaticInstanceMeshData> StaticInstancedMeshRenderer::getInstanceBuffers() const {
	return _data.get();
}
