#include "include/MathLib.h"
#include <algorithm>

float radianFromDegree(const float degree){
	return degree * (M_PI / 180.0f);
}

float clamp(float value, float min, float max){
	return std::min(std::max(value, min), max);
}
