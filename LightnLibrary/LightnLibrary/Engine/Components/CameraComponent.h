#pragma once

#include <LMath.h>
#include <Util/RefPtr.h>
#include <Components/Component.h>
#include <Renderer/Camera.h>

class CameraComponent :public Component{

public:

	CameraComponent();

	~CameraComponent();

	//�ˉe�s���ݒ�
	void setProjection(float fov, float aspect, float minZ, float maxZ);

	//�J�����p�̃r���[�s����擾
	Matrix4 cameraMatrix() const;

	//�ˉe�s����擾
	Matrix4 mtxProj() const;

	//FOV��ݒ�(������)
	void setFov(float fov);

	void setNearClip(float value);

	void setFarClip(float value);

	float nearClip() const;

	float farClip() const;

	float fov() const;

	static RefPtr<CameraComponent> mainCamera;

	RefPtr<Camera> camera();

private:

	Camera _camera;
};