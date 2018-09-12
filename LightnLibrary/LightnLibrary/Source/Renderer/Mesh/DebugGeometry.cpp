#include <Renderer/Mesh/DebugGeometry.h>
#include <Renderer/RendererUtil.h>
#include <Renderer/DrawSettings.h>
#include <ThirdParty/ImGui/imgui.h>

void DebugGeomtryRenderer::initialize(ComPtr<ID3D11Device>& device)
{
	//頂点インプットレイアウトをインスタンス描画ように設定
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	RendererUtil::createVertexShader("DebugGeometry_vs.cso", _vertexShader, layout, ARRAYSIZE(layout), _vertexLayout, device);
	RendererUtil::createPixelShader("DebugGeometry_ps.cso", _pixelShader, device);
	RendererUtil::createConstantBuffer(_constantBuffer, sizeof(MeshConstantBuffer), device);

	const Vector3 vLine[] = { {0,0,0},{0,1,0} };
	const int iLine[] = { 0,1 };
	const Vector3 vBox[] = {
		{ -0.5f, 0.5f, 0.5f },
		{  0.5f, 0.5f, 0.5f },
		{ -0.5f, 0.5f,-0.5f },
		{  0.5f, 0.5f,-0.5f },
		{ -0.5f,-0.5f, 0.5f },
		{  0.5f,-0.5f, 0.5f },
		{ -0.5f,-0.5f,-0.5f },
		{  0.5f,-0.5f,-0.5f } };

	const int iBox[] = {
		0, 1, 1, 3, 3, 2, 2, 0, 0, 4, 4, 5,
		5, 7, 7, 6, 6, 4, 6, 2, 7, 3, 5, 1 };

	const int u_max = 20;
	const int v_max = u_max / 2;
	int vertex_num = u_max * (v_max + 1);
	int index_num = 2 * v_max * (u_max + 1);

	std::vector<int> iSphere(index_num);
	std::vector<Vector3> vSphere(vertex_num);
	for (int v = 0; v <= v_max; v++) {
		for (int u = 0; u < u_max; u++) {
			float theta = radianFromDegree(180.0f * v / v_max);
			float phi = radianFromDegree(360.0f * u / u_max);
			float x = sin(theta) * cos(phi);
			float y = cos(theta);
			float z = sin(theta) * sin(phi);
			vSphere[u_max * v + u] = Vector3(x, y, z) / 2.0f;
		}
	}

	int i = 0;
	for (int v = 0; v < v_max; v++) {
		for (int u = 0; u <= u_max; u++) {
			if (u == u_max) {
				iSphere[i] = v * u_max;
				i++;
				iSphere[i] = (v + 1) * u_max;
			}
			else {
				iSphere[i] = (v * u_max) + u;
				i++;
				iSphere[i] = iSphere[i - 1] + u_max;
			}
			i++;
		}
	}

	_sphereMesh = std::make_unique<DebugGeometry>();
	_sphereMesh->initialize(vSphere.data(), static_cast<uint32>(vSphere.size() * sizeof(Vector3)), iSphere.data(), static_cast<uint32>(iSphere.size()), device);

	_boxMesh = std::make_unique<DebugGeometry>();
	_boxMesh->initialize(vBox, 8 * sizeof(Vector3), iBox, 24, device);

	_lineMesh = std::make_unique<DebugGeometry>();
	_lineMesh->initialize(vLine, 2 * sizeof(Vector3), iLine, 2, device);

	D3D11_RASTERIZER_DESC rasterizerDesc =
	{
		D3D11_FILL_WIREFRAME, // ワイヤーフレーム
		D3D11_CULL_FRONT,      // 裏面ポリゴンをカリングします
		FALSE,
		0,
		0.0f,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE
	};

	device->CreateRasterizerState(&rasterizerDesc, _rasterizerState.ReleaseAndGetAddressOf());
}

void DebugGeomtryRenderer::draw(const DebugDrawList & sphere, const DebugDrawList & box, const DebugDrawList & line, const DrawSettings & drawSettings)
{

	assert(sphere.size() < MAX_INSTANCE_NUM);
	assert(box.size() < MAX_INSTANCE_NUM);
	assert(line.size() < MAX_INSTANCE_NUM);

	auto deviceContext = drawSettings.deviceContext;
	MeshConstantBuffer constantBuffer = RendererUtil::getConstantBuffer(Matrix4::identity, drawSettings.camera);
	deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, NULL, &constantBuffer, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());

	deviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
	deviceContext->RSSetState(_rasterizerState.Get());

	//使用シェーダーをセット
	deviceContext->VSSetShader(_vertexShader.Get(), NULL, 0);

	//頂点インプットレイアウトをセット
	deviceContext->IASetInputLayout(_vertexLayout.Get());

	//ポリゴンの描画ルールをセット
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	deviceContext->PSSetShader(_pixelShader.Get(), NULL, 0);

	/*ImGui::Begin("DebugSphere");
	static float sTs = 1;
	static float tXs = 0;
	static float tYs = 0;
	static float tZs = 0;

	ImGui::SliderFloat("sX", &sTs, -1.0f, 10.0f);
	ImGui::SliderFloat("tX", &tXs, -10.0f, 10.0f);
	ImGui::SliderFloat("tY", &tYs, -10.0f, 10.0f);
	ImGui::SliderFloat("tZ", &tZs, -10.0f, 10.0f);
	ImGui::End();*/

	if (!sphere.empty()) {
		_sphereMesh->draw(drawSettings, sphere.data(), static_cast<uint32>(sphere.size()));
	}

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	/*ImGui::Begin("DebugBox");
	static float sXb = 1;
	static float sYb = 1;
	static float sZb = 1;
	static float rXb = 0;
	static float rYb = 0;
	static float rZb = 0;
	static float tXb = 0;
	static float tYb = 0;
	static float tZb = 0;

	ImGui::SliderFloat("sX", &sXb, -1.0f, 10.0f);
	ImGui::SliderFloat("sY", &sYb, -1.0f, 10.0f);
	ImGui::SliderFloat("sZ", &sZb, -1.0f, 10.0f);
	ImGui::SliderFloat("rX", &rXb, -90.0f, 90.0f);
	ImGui::SliderFloat("rY", &rYb, -90.0f, 90.0f);
	ImGui::SliderFloat("rZ", &rZb, -90.0f, 90.0f);
	ImGui::SliderFloat("tX", &tXb, -10.0f, 10.0f);
	ImGui::SliderFloat("tY", &tYb, -10.0f, 10.0f);
	ImGui::SliderFloat("tZ", &tZb, -10.0f, 10.0f);
	ImGui::End();*/

	if (!box.empty()) {
		_boxMesh->draw(drawSettings, box.data(), static_cast<uint32>(box.size()));
	}

	/*ImGui::Begin("DebugLine");
	static float sX = 0;
	static float sY = 0;
	static float sZ = 0;
	static float eX = 0;
	static float eY = 1;
	static float eZ = 0;

	ImGui::SliderFloat("sX", &sX, -10.0f, 10.0f);
	ImGui::SliderFloat("sY", &sY, -10.0f, 10.0f);
	ImGui::SliderFloat("sZ", &sZ, -10.0f, 10.0f);
	ImGui::SliderFloat("eX", &eX, -10.0f, 10.0f);
	ImGui::SliderFloat("eY", &eY, -10.0f, 10.0f);
	ImGui::SliderFloat("eZ", &eZ, -10.0f, 10.0f);
	ImGui::End();

	Vector3 startToEnd = Vector3(eX, eY, eZ) - Vector3(sX, sY, sZ);
	float length = startToEnd.length();

	Matrix4 mtxRotate = Matrix4::rotateAxis(Vector3::normalize(startToEnd));
	Matrix4 mtxTranslate = Matrix4::translateXYZ(sX, sY, sZ);
	Matrix4 mtxScale = Matrix4::scaleXYZ(length, length, length);

	constantBuffer.mtxWorld = Matrix4::transpose(mtxScale.multiply(mtxRotate.multiply(mtxTranslate)));*/
	if (!line.empty()) {
		_lineMesh->draw(drawSettings, line.data(), static_cast<uint32>(line.size()));
	}
	
	deviceContext->RSSetState(NULL);
}

void DebugGeometry::initialize(const void* vertices, uint32 vSize, const void* indices, uint32 iSize, ComPtr<ID3D11Device>& device)
{
	RendererUtil::createVertexBuffer(vertices, vSize, _vertexBuffer, device);
	RendererUtil::createVertexBuffer(nullptr, sizeof(DebugGeometoryInfo)*MAX_INSTANCE_NUM, _debugGeometryInfoBuffer, device);
	RendererUtil::createIndexBuffer(indices, iSize, _indexBuffer, device);
	_indexCount = iSize;
}

void DebugGeometry::draw(const DrawSettings & drawSettings,const DebugGeometoryInfo* info, uint32 instanceCount)
{
	auto deviceContext = drawSettings.deviceContext;

	deviceContext->UpdateSubresource(_debugGeometryInfoBuffer.Get(), 0, 0, info, 0, 0);
	
	ID3D11Buffer* insBuf[2] = { _vertexBuffer.Get(), _debugGeometryInfoBuffer.Get() };
	const uint32 stride[2] = { sizeof(Vector3) , sizeof(DebugGeometoryInfo) };
	const uint32 offset[2] = { 0,0 };
	deviceContext->IASetVertexBuffers(0, 2, insBuf, stride, offset);
	deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	deviceContext->DrawIndexedInstanced(_indexCount, instanceCount, 0, 0, 0);
}
