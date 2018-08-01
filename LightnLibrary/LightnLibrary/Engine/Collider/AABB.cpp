#include "AABB.h"


AABB::AABB()
{
}

AABB::AABB(const Vector3 & min, const Vector3 & max) :min{ min }, max{ max }
{
}

bool AABB::intersectAABB(const AABB & b1, const AABB & b2)
{
	if (b1.min.x > b2.max.x) return false;
	if (b1.max.x < b2.min.x) return false;
	if (b1.min.y > b2.max.y) return false;
	if (b1.max.y < b2.min.y) return false;
	if (b1.min.z > b2.max.z) return false;
	if (b1.max.z < b2.min.z) return false;
	return false;
}

bool AABB::intersectAABB(const AABB & b2)
{
	return intersectAABB(*this, b2);
}

Vector3 AABB::extent() const
{
	return this->size() / 2;
}

Vector3 AABB::size() const {
	return max - min;
}

Vector3 AABB::center() const
{
	return (max + min) / 2;
}

AABB AABB::createTransformMatrix(const Matrix4 & matrix) const
{
	const Vector3 min = this->min;
	const Vector3 size = this->size();

	const Vector3 points[8] = {
		min + Vector3(size.x,size.y,size.z),
		min + Vector3(size.x,size.y,0),
		min + Vector3(size.x,0,0),
		min + Vector3(0,0,0),
		min + Vector3(size.x,0,size.z),
		min + Vector3(0,0,size.z),
		min + Vector3(0,size.y,size.z),
		min + Vector3(0,size.y,0),
	};

	Matrix4 sr = matrix;
	sr.m[3][0] = 0;
	sr.m[3][1] = 0;
	sr.m[3][2] = 0;

	Vector3 resultMin;
	Vector3 resultMax;

	for (int i = 0; i < 8; ++i) {
		const Vector3 p = Matrix4::transform(points[i], sr);

		if (i == 0) {
			resultMin = p;
			resultMax = p;
			continue;
		}

		if (p.x < resultMin.x) { resultMin.x = p.x; }
		if (p.y < resultMin.y) { resultMin.y = p.y; }
		if (p.z < resultMin.z) { resultMin.z = p.z; }

		if (p.x > resultMax.x) { resultMax.x = p.x; }
		if (p.y > resultMax.y) { resultMax.y = p.y; }
		if (p.z > resultMax.z) { resultMax.z = p.z; }
	}

	return AABB(resultMin, resultMax);
}

void AABB::translate(const Vector3 & v) {
	this->min += v;
	this->max += v;
}
