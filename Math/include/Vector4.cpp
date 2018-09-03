#include "Vector4.h"

Vector4 operator+(const Vector4 & v) {
	return v;
}

Vector4 operator-(const Vector4 & v) {
	return	Vector4(-v.x, -v.y, -v.z, -v.w);
}
