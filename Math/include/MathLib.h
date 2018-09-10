#pragma once

#include <cmath>

const float EPSILON = 0.000001f;
const float M_PI = 3.141592654f;
const float M_2PI = 6.283185307f;
const float M_1DIVPI = 0.318309886f;
const float M_1DIV2PI = 0.159154943f;
const float M_PIDIV2 = 1.570796327f;
const float M_PIDIV4 = 0.785398163f;

//“x”‚©‚çƒ‰ƒWƒAƒ“‚É•ÏŠ·
float radianFromDegree(const float degree);

float clamp(float value, float min, float max);
float lerp(float value, float min, float max);