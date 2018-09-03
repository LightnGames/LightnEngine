#include "include/Quaternion.h"
#include "include/MathLib.h"

const Quaternion Quaternion::identity{ 0.0f,0.0f,0.0f,1.0f };

Quaternion::Quaternion() :x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, w{ 1.0f }{
}

Quaternion::Quaternion(float x, float y, float z, float w) : x{ x }, y{ y }, z{ z }, w{ w }{
}

Quaternion::Quaternion(const Vector3 & axis, float angle){
	const float sin = std::sin(angle / 2.0f);
	x = sin*axis.x;
	y = sin*axis.y;
	z = sin*axis.z;
	w = std::cos(angle / 2.0f);
}

Quaternion Quaternion::slerp(const Quaternion q1, const Quaternion & q2,float t){
	float cos = dot(q1, q2);
	Quaternion t2 = q2;
	if(cos < 0.0f){
		cos = -cos;
		t2 = -q2;
	}

	float k0 = 1.0f - t;
	float k1 = t;
	if(( 1.0f - cos ) > 0.001f){
		float theta = (float)std::acos(cos);
		k0 = (float)( std::sin(theta*k0) / std::sin(theta) );
		k1 = (float)( std::sin(theta*k1) / std::sin(theta) );
	}
	return q1*k0 + t2*k1;
}

Quaternion Quaternion::euler(const Vector3 & euler, bool valueIsRadian) {

	const Vector3 rad = valueIsRadian ? euler : (euler * M_PI / 180);
	Quaternion q;

	double cy = cos(rad.z * 0.5);
	double sy = sin(rad.z * 0.5);
	double cr = cos(rad.x * 0.5);
	double sr = sin(rad.x * 0.5);
	double cp = cos(rad.y * 0.5);
	double sp = sin(rad.y * 0.5);

	q.w = cy * cr * cp + sy * sr * sp;
	q.x = cy * sr * cp - sy * cr * sp;
	q.y = cy * cr * sp + sy * sr * cp;
	q.z = sy * cr * cp - cy * sr * sp;
	return q;
}

Vector3 Quaternion::rotVector(const Quaternion & q, const Vector3 & v)
{

	// nVidia SDK implementation
	Vector3 uv, uuv;
	Vector3 qvec(q.x, q.y, q.z);
	//uv = qvec.crossProduct(v);
	uv = Vector3::cross(qvec, v);
	//uuv = qvec.crossProduct(uv);
	uuv = Vector3::cross(qvec, uv);
	uv *= (2.0f * q.w);
	uuv *= 2.0f;

	return v + uv + uuv;

	const Vector3 u(q.x, q.y, q.z);
	float s = q.w;

	Vector3 result = 2.0f *Vector3::dot(u, v) * u
		+ (s*s - Vector3::dot(u, u)) * v
		+ 2.0f * s * Vector3::cross(u, v);

	return result;
}

//行列をかますのは不毛？
#include "include/Matrix4.h"
Quaternion Quaternion::lookRotation(const Vector3 & direction, const Vector3 & up) {

	Vector3 leftDir = direction;
	leftDir.x *= -1;
	Vector3 xaxis = Vector3::cross(up, leftDir);
	xaxis.normalize();

	Vector3 yaxis = Vector3::cross(leftDir, xaxis);
	yaxis.normalize();

	Matrix4 mat;

	mat.m[0][0] = xaxis.x;
	mat.m[0][1] = yaxis.x;
	mat.m[0][2] = leftDir.x;

	mat.m[1][0] = xaxis.y;
	mat.m[1][1] = yaxis.y;
	mat.m[1][2] = leftDir.y;

	mat.m[2][0] = xaxis.z;
	mat.m[2][1] = yaxis.z;
	mat.m[2][2] = leftDir.z;

	return mat.rotation();
}

float Quaternion::dot(const Quaternion & q1, const Quaternion & q2){
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

Quaternion Quaternion::inverse() const {

	const float fNorm = w * w + x * x + y * y + z * z;

	//不正なクォータニオン
	if (fNorm == 0.0f) {
		return Quaternion::identity;
	}

	const float fInvNorm = 1.0f / fNorm;
	return Quaternion(w*fInvNorm, -x * fInvNorm, -y * fInvNorm, -z * fInvNorm);
}

Vector3 Quaternion::toEulerAngle() const {

	Vector3 result;
	// roll (x-axis rotation)
	double sinr = +2.0 * (w * x + y * z);
	double cosr = +1.0 - 2.0 * (x * x + y * y);
	result.x = atan2(sinr, cosr);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (w * y - z * x);
	if (fabs(sinp) >= 1) {
		result.y = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	}
	else {
		result.y = asin(sinp);
	}

	// yaw (z-axis rotation)
	double siny = +2.0 * (w * z + x * y);
	double cosy = +1.0 - 2.0 * (y * y + z * z);
	result.z = atan2(siny, cosy);

	return result;
}

float Quaternion::getRoll(bool reprojectAxis) const {
	if (reprojectAxis) {
		// roll = atan2(localx.y, localx.x)
		// pick parts of xAxis() implementation that we need
		//			Real fTx  = 2.0*x;
		float fTy = 2.0f*y;
		float fTz = 2.0f*z;
		float fTwz = fTz * w;
		float fTxy = fTy * x;
		float fTyy = fTy * y;
		float fTzz = fTz * z;

		// Vector3(1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);

		return atan2(fTxy + fTwz, 1.0f - (fTyy + fTzz));

	}
	else {
		return atan2(2 * (x*y + w * z), w*w + x * x - y * y - z * z);
	}
}
//-----------------------------------------------------------------------
float Quaternion::getPitch(bool reprojectAxis) const {
	if (reprojectAxis) {
		// pitch = atan2(localy.z, localy.y)
		// pick parts of yAxis() implementation that we need
		float fTx = 2.0f*x;
		//			Real fTy  = 2.0f*y;
		float fTz = 2.0f*z;
		float fTwx = fTx * w;
		float fTxx = fTx * x;
		float fTyz = fTz * y;
		float fTzz = fTz * z;

		// Vector3(fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx);
		return atan2(fTyz + fTwx, 1.0f - (fTxx + fTzz));
	}
	else {
		// internal version
		return atan2(2 * (y*z + w * x), w*w - x * x - y * y + z * z);
	}
}
//-----------------------------------------------------------------------
float Quaternion::getYaw(bool reprojectAxis) const {
	if (reprojectAxis) {
		// yaw = atan2(localz.x, localz.z)
		// pick parts of zAxis() implementation that we need
		float fTx = 2.0f*x;
		float fTy = 2.0f*y;
		float fTz = 2.0f*z;
		float fTwy = fTy * w;
		float fTxx = fTx * x;
		float fTxz = fTz * x;
		float fTyy = fTy * y;

		// Vector3(fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy));

		return atan2(fTxz + fTwy, 1.0f - (fTxx + fTyy));

	}
	else {
		// internal version
		return asin(-2 * (x*z - w * y));
	}
}

Quaternion operator-(const Quaternion & q){
	Quaternion result = { -q.x, -q.y, -q.z, -q.w };
	return result;
}

Quaternion & operator+=(Quaternion & q1, const Quaternion & q2){
	q1.x += q2.x;
	q1.y += q2.y;
	q1.z += q2.z;
	q1.w += q2.w;
	return q1;
}

Quaternion & operator-=(Quaternion & q1, const Quaternion & q2){
	q1.x -= q2.x;
	q1.y -= q2.y;
	q1.z -= q2.z;
	q1.w -= q2.w;
	return q1;
}

Quaternion & operator*=(Quaternion & q1, const Quaternion & q2){
	Quaternion result = {
		q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
		-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
		q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
		-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w
	};

	q1 = result;
	return q1;
}

Quaternion & operator*=(Quaternion & q1, float s){
	q1.x *= s;
	q1.y *= s;
	q1.z *= s;
	q1.w *= s;
	return q1;
}

Quaternion & operator/=(Quaternion & q, float s){
	return q *= 1.0f / s;
}

Quaternion operator+(const Quaternion & q1, const Quaternion & q2){
	Quaternion result = q1;
	return result += q2;
}

Quaternion operator-(const Quaternion & q1, const Quaternion & q2){
	Quaternion result = q1;
	return result -= q2;
}

Quaternion operator*(const Quaternion & q1, const Quaternion & q2){
	Quaternion result = q1;
	return result *= q2;
}

Quaternion operator*(const Quaternion & q, float s){
	Quaternion result = q;
	return result *= s;
}

Quaternion operator*(float s, const Quaternion & q){
	Quaternion result = q;
	return result *= s;
}

Quaternion operator/(const Quaternion & q, float s){
	Quaternion result = q;
	return result /= s;
}
