#pragma once

#include <LMath.h>

struct TileBasedPointLightType {
	Vector3 positionView;
	float attenuationBegin;
	Vector3 color;
	float attenuationEnd;
};

struct TileBasedSpotLightType {
	Vector3 positionView;
	float attenuationBegin;
	Vector3 color;
	float attenuationEnd;
	Vector4 direction;
};