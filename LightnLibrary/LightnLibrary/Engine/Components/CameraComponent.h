#pragma once

#include <LMath.h>
#include <Util/RefPtr.h>
#include <Components/Component.h>

class CameraComponent :public Component{

public:

	CameraComponent();

	~CameraComponent();

	//射影行列を設定
	void setProjection(float fov, float aspect, float minZ, float maxZ);

	//カメラ用のビュー行列を取得
	Matrix4 cameraMatrix() const;

	//射影行列を取得
	Matrix4 mtxProj() const;

	//FOVを設定(未実装)
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