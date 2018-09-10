#include "Vector4.h"

Vector4 operator+(const Vector4 & v) {
	return v;
}

Vector4 operator-(const Vector4 & v) {
	return	Vector4(-v.x, -v.y, -v.z, -v.w);
}

Vector4 & operator*=(Vector4 & v, float s) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	v.w *= s;
	return v;
}
