#include <GfxCore/impl/ViewSystemImpl.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <Core/Application.h>

ViewSystemImpl _viewSystem;

void ViewSystemImpl::initialize() {
	_mainView.initialize();
	_debugFixedView.initialize();
}

void ViewSystemImpl::terminate() {
	_mainView.terminate();
	_debugFixedView.terminate();
}

void ViewSystemImpl::update() {
	enum CameraMode : s32 {
		CAMERA_MODE_DEFAULT = 0,
		CAMERA_MODE_MAIN,
		CAMERA_MODE_CULL
	};

	struct CameraInfo {
		Vector3 position = Vector3(-7, 5, -30);
		Vector3 cameraAngle = Vector3(0, 0, 0);
		f32 fov = 1.0472f;
		f32 depthPrePassDistance = 20.0f;
		s32 _cameraMode = 0;
	};

	static bool autoTranslate = false;
	static bool autoRotate = false;
	static f32 autoTranslateTime = 0.0f;
	static f32 autoRotateTime = 0.0f;
	auto debug = DebugWindow::StartWindow<CameraInfo>("CameraInfo");
	DebugGui::DragFloat("depth pre pass distance", &debug.depthPrePassDistance);

	const char* cameraModes[] = { "Default", "Main", "Cull" };
	DebugGui::Combo("Camera Mode", reinterpret_cast<s32*>(&debug._cameraMode), cameraModes, LTN_COUNTOF(cameraModes));

	DebugGui::DragFloat3("position", &debug.position._x, 0.1f);
	DebugGui::SliderAngle("fov", &debug.fov, 0.1f);

	f32 aspectRate = _mainView._viewPort._width / _mainView._viewPort._height;
	if (DebugGui::BeginTabBar("ViewSystemTabBar")) {
		if (DebugGui::BeginTabItem("Fixed View")) {
			DebugGui::Image(_debugFixedView._hdrSrv._gpuHandle, Vector2(200 * aspectRate, 200));
			DebugGui::EndTabItem();
		}
		DebugGui::EndTabBar();
	}

	const char* autoRotateLabels[] = { "Auto Rotate", "Stop Auto Rotate" };
	if(DebugGui::Button(autoRotateLabels[autoRotate])) {
		if (autoRotate) {
			autoRotateTime = 0.0f;
		}
		autoRotate = !autoRotate;
	}

	const char* autoTranslateLabels[] = { "Auto Translate", "Stop Auto Translate" };
	if (DebugGui::Button(autoTranslateLabels[autoTranslate])) {
		if (autoTranslate) {
			autoTranslateTime = 0.0f;
		}
		autoTranslate = !autoTranslate;
	}

	DebugWindow::End();

	// マウス右クリックでの画面回転
	static Vector2 prevMousePosition = Vector2(0, 0);
	InputSystem* inputSystem = InputSystem::Get();
	Vector2 currentMousePosition = inputSystem->getMousePosition();
	if (inputSystem->getKey(InputSystem::KEY_CODE_RBUTTON)) {
		constexpr f32 SCALE = 0.005f;
		Vector2 distance = currentMousePosition - prevMousePosition;
		debug.cameraAngle._x += distance._y * SCALE;
		debug.cameraAngle._y += distance._x * SCALE;
	}
	prevMousePosition = currentMousePosition;

	Matrix4 cameraRotate = Matrix4::rotate(debug.cameraAngle);
	if (autoRotate) {
		autoRotateTime += 0.01f;
		cameraRotate = cameraRotate * Matrix4::rotateY(autoRotateTime);
	}

	Vector3 rightDirection = cameraRotate.mv[0].toVector3();
	Vector3 upDirection = cameraRotate.mv[1].toVector3();
	Vector3 forwardDirection = cameraRotate.mv[2].toVector3();

	// W/A/S/D キーボードによる移動
	Vector3 moveDirection = Vector3::Zero;
	if (inputSystem->getKey(InputSystem::KEY_CODE_W)) {
		moveDirection += forwardDirection;
	}

	if (inputSystem->getKey(InputSystem::KEY_CODE_S)) {
		moveDirection -= forwardDirection;
	}

	if (inputSystem->getKey(InputSystem::KEY_CODE_D)) {
		moveDirection += rightDirection;
	}

	if (inputSystem->getKey(InputSystem::KEY_CODE_A)) {
		moveDirection -= rightDirection;
	}

	if (inputSystem->getKey(InputSystem::KEY_CODE_E)) {
		moveDirection += upDirection;
	}

	if (inputSystem->getKey(InputSystem::KEY_CODE_Q)) {
		moveDirection -= upDirection;
	}

	constexpr f32 DEBUG_CAMERA_MOVE_SPEED = 0.15f;
	moveDirection = Vector3::normalize(moveDirection) * DEBUG_CAMERA_MOVE_SPEED;
	debug.position += moveDirection;

	Vector3 cameraPosition = debug.position;
	if (autoTranslate) {
		autoTranslateTime += 0.015f;
		cameraPosition += Vector3::Right * std::sin(autoTranslateTime) * 5;
	}

	_isEnabledDebugFixedView = debug._cameraMode != CAMERA_MODE_DEFAULT;

	// メインビュー用定数バッファ
	f32 farClip = 300;
	f32 nearClip = 0.1f;
	f32 fovHalfTan = tanf(debug.fov / 2.0f);
	Matrix4 viewMatrix = cameraRotate * Matrix4::translate(cameraPosition);
	Matrix4 projectionMatrix = Matrix4::perspectiveFovLH(debug.fov, aspectRate, nearClip, farClip);
	ViewConstant viewConstant;
	viewConstant._matrixView = viewMatrix.inverse().transpose();
	viewConstant._matrixProj = projectionMatrix.transpose();
	viewConstant._matrixViewProj = viewConstant._matrixProj * viewConstant._matrixView;
	viewConstant._position = cameraPosition.getFloat3();
	viewConstant._nearAndFarClip._x = nearClip;
	viewConstant._nearAndFarClip._y = farClip;
	viewConstant._halfFovTanX = fovHalfTan * aspectRate;
	viewConstant._halfFovTanY = fovHalfTan;
	viewConstant._viewPortSizeX = static_cast<u32>(_mainView._viewPort._width);
	viewConstant._viewPortSizeY = static_cast<u32>(_mainView._viewPort._height);
	viewConstant._upDirection = Matrix4::transformNormal(Vector3::Up, cameraRotate).getFloat3();

	// メインビュー用　フラスタム平面
	Vector3 sideForward = Vector3::Forward * fovHalfTan * aspectRate;
	Vector3 forward = Vector3::Forward * fovHalfTan;
	Vector3 rightNormal = Matrix4::transformNormal(Vector3::Right + sideForward, cameraRotate).getNormalize();
	Vector3 leftNormal = Matrix4::transformNormal(-Vector3::Right + sideForward, cameraRotate).getNormalize();
	Vector3 buttomNormal = Matrix4::transformNormal(Vector3::Up + forward, cameraRotate).getNormalize();
	Vector3 topNormal = Matrix4::transformNormal(-Vector3::Up + forward, cameraRotate).getNormalize();
	Vector3 nearNormal = Matrix4::transformNormal(Vector3::Forward, cameraRotate).getNormalize();
	Vector3 farNormal = Matrix4::transformNormal(-Vector3::Forward, cameraRotate).getNormalize();
	viewConstant._frustumPlanes[0] = Float4(rightNormal._x, rightNormal._y, rightNormal._z, Vector3::dot(rightNormal, cameraPosition));
	viewConstant._frustumPlanes[1] = Float4(leftNormal._x, leftNormal._y, leftNormal._z, Vector3::dot(leftNormal, cameraPosition));
	viewConstant._frustumPlanes[2] = Float4(buttomNormal._x, buttomNormal._y, buttomNormal._z, Vector3::dot(buttomNormal, cameraPosition));
	viewConstant._frustumPlanes[3] = Float4(topNormal._x, topNormal._y, topNormal._z, Vector3::dot(topNormal, cameraPosition));
	viewConstant._frustumPlanes[4] = Float4(nearNormal._x, nearNormal._y, nearNormal._z, Vector3::dot(nearNormal, cameraPosition) + nearClip);
	viewConstant._frustumPlanes[5] = Float4(farNormal._x, farNormal._y, farNormal._z, Vector3::dot(farNormal, cameraPosition) - farClip);

	// CPU 側で参照する用の定数バッファ情報
	ViewConstantInfo viewConstantInfo;
	viewConstantInfo._nearClip = nearClip;
	viewConstantInfo._farClip = farClip;
	viewConstantInfo._viewMatrix = viewMatrix;
	viewConstantInfo._projectionMatrix = projectionMatrix;

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	if (debug._cameraMode == CAMERA_MODE_CULL || debug._cameraMode == CAMERA_MODE_DEFAULT) {
		ViewConstant* debugFixedViewConstant = vramUpdater->enqueueUpdate<ViewConstant>(&_debugFixedView._viewInfoBuffer, 0);
		memcpy(debugFixedViewConstant, &viewConstant, sizeof(ViewConstant));

		ViewConstant* debugFixedViewCullingViewConstant = vramUpdater->enqueueUpdate<ViewConstant>(&_debugFixedView._cullingViewInfoBuffer, 0);
		memcpy(debugFixedViewCullingViewConstant, &viewConstant, sizeof(ViewConstant));

		ViewConstant* cullingViewConstant = vramUpdater->enqueueUpdate<ViewConstant>(&_mainView._cullingViewInfoBuffer, 0);
		memcpy(cullingViewConstant, &viewConstant, sizeof(ViewConstant));

		_debugFixedView._mainViewConstantInfo = viewConstantInfo;
		_debugFixedView._cullingViewConstantInfo = viewConstantInfo;
		_mainView._cullingViewConstantInfo = viewConstantInfo;
	}

	if (debug._cameraMode == CAMERA_MODE_MAIN || debug._cameraMode == CAMERA_MODE_DEFAULT) {
		ViewConstant* mainViewConstant = vramUpdater->enqueueUpdate<ViewConstant>(&_mainView._viewInfoBuffer, 0);
		memcpy(mainViewConstant, &viewConstant, sizeof(ViewConstant));

		_mainView._mainViewConstantInfo = viewConstantInfo;
	}

	// デプスプリパス用ビュー定数バッファ更新
	if (debug._cameraMode == CAMERA_MODE_MAIN || debug._cameraMode == CAMERA_MODE_DEFAULT) {
		f32 depthPrePassFarClip = debug.depthPrePassDistance;
		ViewConstant* depthPrePassViewConstant = vramUpdater->enqueueUpdate<ViewConstant>(&_mainView._depthPrePassViewInfoBuffer, 0);
		memcpy(depthPrePassViewConstant, &viewConstant, sizeof(ViewConstant));
		depthPrePassViewConstant->_frustumPlanes[5] = Float4(farNormal._x, farNormal._y, farNormal._z, Vector3::dot(farNormal, debug.position) - depthPrePassFarClip);
		depthPrePassViewConstant->_nearAndFarClip._y = depthPrePassFarClip;
	}
}

void ViewSystemImpl::processDeletion() {
}

ViewSystemImpl* ViewSystemImpl::Get() {
	return &_viewSystem;
}

void ViewInfo::initialize() {
	GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
	Device* device = graphicsSystem->getDevice();
	DescriptorHeapAllocator* allocator = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator();
	Application* app = ApplicationSystem::Get()->getApplication();
	u32 screenWidth = app->getScreenWidth();
	u32 screenHeight = app->getScreenHeight();

	// view ports
	{
		_viewPort._width = static_cast<f32>(screenWidth);
		_viewPort._height = static_cast<f32>(screenHeight);
		_viewPort._maxDepth = 1.0f;
		_scissorRect._right = static_cast<l32>(screenWidth);
		_scissorRect._bottom = static_cast<l32>(screenHeight);
	}

	// cbv
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = GetConstantBufferAligned(sizeof(ViewConstant));
		desc._initialState = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		desc._device = device;
		_viewInfoBuffer.initialize(desc);
		_viewInfoBuffer.setDebugName("Main View Info");

		_cullingViewInfoBuffer.initialize(desc);
		_cullingViewInfoBuffer.setDebugName("Culling View Info");

		_depthPrePassViewInfoBuffer.initialize(desc);
		_depthPrePassViewInfoBuffer.setDebugName("Depth Pre Pass View Info");

		DescriptorHeapAllocator* allocater = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator();
		_viewInfoCbv = allocater->allocateDescriptors(1);
		_cullingViewInfoCbv = allocater->allocateDescriptors(1);
		_depthPrePassViewInfoCbv = allocater->allocateDescriptors(1);
		device->createConstantBufferView(_viewInfoBuffer.getConstantBufferViewDesc(), _viewInfoCbv._cpuHandle);
		device->createConstantBufferView(_cullingViewInfoBuffer.getConstantBufferViewDesc(), _cullingViewInfoCbv._cpuHandle);
		device->createConstantBufferView(_depthPrePassViewInfoBuffer.getConstantBufferViewDesc(), _depthPrePassViewInfoCbv._cpuHandle);
	}

	// depth stencil texture
	{

		ClearValue depthOptimizedClearValue = {};
		depthOptimizedClearValue._format = FORMAT_D32_FLOAT;
		depthOptimizedClearValue._depthStencil._depth = 1.0f;
		depthOptimizedClearValue._depthStencil._stencil = 0;

		GpuTextureDesc desc = {};
		desc._device = device;
		desc._format = FORMAT_D32_FLOAT;
		desc._optimizedClearValue = &depthOptimizedClearValue;
		desc._width = screenWidth;
		desc._height = screenHeight;
		desc._flags = RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		desc._initialState = RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		_depthTexture.initialize(desc);
		_depthTexture.setDebugName("Main View Depth");

		ShaderResourceViewDesc srvDesc = {};
		srvDesc._format = FORMAT_R32_FLOAT;
		srvDesc._viewDimension = SRV_DIMENSION_TEXTURE2D;
		srvDesc._texture2D._mipLevels = 1;
		_depthSrv = allocator->allocateDescriptors(1);
		device->createShaderResourceView(_depthTexture.getResource(), &srvDesc, _depthSrv._cpuHandle);
	}

	// hdr texture
	{
		ClearValue depthOptimizedClearValue = {};
		depthOptimizedClearValue._format = BACK_BUFFER_FORMAT;

		GpuTextureDesc desc = {};
		desc._device = device;
		desc._format = BACK_BUFFER_FORMAT;
		desc._optimizedClearValue = &depthOptimizedClearValue;
		desc._width = screenWidth;
		desc._height = screenHeight;
		desc._flags = RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc._initialState = RESOURCE_STATE_RENDER_TARGET;
		_hdrTexture.initialize(desc);
		_hdrTexture.setDebugName("Main View HDR");
	}

	// descriptors
	{
		_depthDsv = graphicsSystem->getDsvGpuDescriptorAllocator()->allocateDescriptors(1);
		device->createDepthStencilView(_depthTexture.getResource(), _depthDsv._cpuHandle);

		_hdrRtv = graphicsSystem->getRtvGpuDescriptorAllocator()->allocateDescriptors(1);
		device->createRenderTargetView(_hdrTexture.getResource(), _hdrRtv._cpuHandle);

		_hdrSrv = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator()->allocateDescriptors(1);
		device->createShaderResourceView(_hdrTexture.getResource(), nullptr, _hdrSrv._cpuHandle);
	}
}

void ViewInfo::terminate() {
	_viewInfoBuffer.terminate();
	_cullingViewInfoBuffer.terminate();
	_depthPrePassViewInfoBuffer.terminate();
	_depthTexture.terminate();
	_hdrTexture.terminate();

	GraphicsSystemImpl* graphicsSystem = GraphicsSystemImpl::Get();
	// srv cbv uav
	{
		DescriptorHeapAllocator* allocator = graphicsSystem->getSrvCbvUavGpuDescriptorAllocator();
		allocator->discardDescriptor(_viewInfoCbv);
		allocator->discardDescriptor(_cullingViewInfoCbv);
		allocator->discardDescriptor(_depthPrePassViewInfoCbv);
		allocator->discardDescriptor(_depthSrv);
	}

	// dsv rtv
	{
		graphicsSystem->getDsvGpuDescriptorAllocator()->discardDescriptor(_depthDsv);
		graphicsSystem->getRtvGpuDescriptorAllocator()->discardDescriptor(_hdrRtv);
		graphicsSystem->getSrvCbvUavGpuDescriptorAllocator()->discardDescriptor(_hdrSrv);
	}
}

void ViewInfo::debugDrawDepth() {
	f32 aspectRate = _viewPort._width / _viewPort._height;
	ResourceDesc desc = _depthTexture.getResourceDesc();
	DebugGui::Image(_depthSrv._gpuHandle, Vector2(200 * aspectRate, 200),
		Vector2(0, 0), Vector2(1, 1), Color4::WHITE, Color4::BLACK, DebugGui::COLOR_CHANNEL_FILTER_R, Vector2(0.95f, 1));
}
