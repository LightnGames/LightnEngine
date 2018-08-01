#pragma once

#include <LMath.h>
#include <Util/RefPtr.h>
#include <Components/Component.h>

class CameraComponent :public Component{

public:

	CameraComponent();

	~CameraComponent();

	//ËesñðÝè
	void setProjection(float fov, float aspect, float minZ, float maxZ);

	//JpÌr[sñðæ¾
	Matrix4 cameraMatrix() const;

	//Ëesñðæ¾
	Matrix4 mtxProj() const;

	//FOVðÝè(¢À)
	void setFov(float fov);

	void setNearClip(float value);

	void setFarClip(float value);

	float nearClip() const;

	float farClip() const;

	float fov() const;

	static RefPtr<CameraComponent> mainCamera;

private:

	Matrix4 _mtxProj;
	float _near;
	float _far;
	float _fov;
};