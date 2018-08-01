#include "CameraComponent.h"
#include <Renderer/GameRenderer.h>
#include <Actor/Actor.h>

RefPtr<CameraComponent> CameraComponent::mainCamera = 0;

CameraComponent::CameraComponent(){
	setFov(45);
}

CameraComponent::~CameraComponent() {
}

void CameraComponent::setProjection(float fov, float aspect, float minZ, float maxZ){
	_mtxProj = Matrix4::perspectiveFovLH(fov, aspect, minZ, maxZ);
	setNearClip(minZ);
	setFarClip(maxZ);
}

//スケールを考慮しないカメラ用のビュー行列を取得
Matrix4 CameraComponent::cameraMatrix() const{

	const Matrix4 localTR = Matrix4::createWorldMatrix(_localTransform.position, _localTransform.rotation, Vector3{ 1,1,1 });
	const Matrix4 parentActorTR = Matrix4::createWorldMatrix(_parentActor->getActorPosition(), _parentActor->getActorRotation(), Vector3{ 1,1,1 });
	const Matrix4 mtxView = Matrix4::multiply(localTR, parentActorTR);

	return mtxView;
}

Matrix4 CameraComponent::mtxProj() const{
	return _mtxProj;
}

void CameraComponent::setFov(float fov)
{
	const auto& screenSize = GameRenderer::instance().screenSize();

	//射影行列
	float fovT = radianFromDegree(fov);
	float aspect = screenSize.x / screenSize.y;
	float minZ = 0.1f;
	float maxZ = 5000.0f;
	setProjection(fovT, aspect, minZ, maxZ);

	_fov = fov;
}

void CameraComponent::setNearClip(float value) {
	_near = value;
}

void CameraComponent::setFarClip(float value) {
	_far = value;
}

float CameraComponent::nearClip() const {
	return _near;
}

float CameraComponent::farClip() const {
	return _far;
}

float CameraComponent::fov() const
{
	return _fov;
}
