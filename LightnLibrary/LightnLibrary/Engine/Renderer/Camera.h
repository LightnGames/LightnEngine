#pragma once

#include <LMath.h>

struct Camera {
	Matrix4 mtxProj;
	Matrix4 mtxView;
	Vector3 position;
	float nearClip;
	float farClip;
	float fov;
};