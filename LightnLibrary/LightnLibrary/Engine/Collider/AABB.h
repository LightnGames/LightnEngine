#pragma once

#include <LMath.h>

class AABB {
public:

	AABB();
	AABB(const Vector3& min, const Vector3& max);

	static bool intersectAABB(const AABB & b1, const AABB & b2);

	bool intersectAABB(const AABB & b2);

	Vector3 extent() const;

	Vector3 size() const;

	Vector3 center() const;

	AABB createTransformMatrix(const Matrix4& matrix) const;

	void translate(const Vector3& v);

	Vector3 min;
	Vector3 max;

};