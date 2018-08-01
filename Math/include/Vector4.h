#pragma once

#include "Vector3.h"

class Vector4{

public:

	Vector4() :x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 0.0f }{
	}

	Vector4(const float x, const float y, const float z, const float w) :x{ x }, y{ y }, z{ z }, w{ w }{
	}

	Vector4(const Vector3& v, const float w = 0.0f) :x{ v.x }, y{ v.y }, z{ v.z }, w{ w }{
	}

	Vector3 toVector3() const{
		return Vector3(x, y, z);
	}

	bool operator == (const Vector4& v){ return x == v.x&&y == v.y&&z == v.z&&w == v.w; }

	float x;
	float y;
	float z;
	float w;
};


