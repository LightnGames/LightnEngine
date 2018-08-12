#include "include/Matrix4.h"
#include "include/MathLib.h"
#include "include/Vector3.h"
#include "include/Quaternion.h"

#include <assert.h>

const Matrix4 Matrix4::identity{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

const Matrix4 Matrix4::textureBias{
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f
};

Matrix4 Matrix4::rotateX(float pitch){

	const float rad = radianFromDegree(pitch);
	const float sin = std::sin(rad);
	const float cos = std::cos(rad);
	Matrix4 result = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos, sin, 0.0f,
		0.0f, -sin, cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return result;

}

Matrix4 Matrix4::rotateY(float yaw){

	const float rad = radianFromDegree(yaw);
	const float sin = std::sin(rad);
	const float cos = std::cos(rad);
	Matrix4 result = {
		cos, 0.0f, -sin, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sin, 0.0f, cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return result;
}

Matrix4 Matrix4::rotateZ(float roll){

	const float rad = radianFromDegree(roll);
	const float sin = std::sin(rad);
	const float cos = std::cos(rad);
	Matrix4 result = {
		cos, sin, 0.0f, 0.0f,
		-sin, cos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return result;
}

Matrix4 Matrix4::rotateXYZ(float pitch, float yaw, float roll){

	Matrix4 p = rotateX(pitch);
	Matrix4 y = rotateY(yaw);
	Matrix4 r = rotateZ(roll);

	return Matrix4::multiply(p, Matrix4::multiply(y, r));
}

Matrix4 Matrix4::rotateAxis(const Vector3 & axis){

	Vector3 forward = Vector3::normalize(axis);
	Vector3 dir = Vector3::cross(Vector3::up, forward);
	float angle = std::acos(Vector3::dot(forward, Vector3::up));

	dir = Vector3::normalize(dir);

	return Matrix4::matrixFromQuaternion(Quaternion(dir, angle));
}

Matrix4 Matrix4::scaleXYZ(float x, float y, float z){
	return Matrix4(
		x,0.0f,0.0f,0.0f,
		0.0f,y,0.0f,0.0f,
		0.0f,0.0f,z,0.0f,
		0.0f,0.0f,0.0f,1.0f);
}

Matrix4 Matrix4::scaleXYZ(const Vector3 & v){
	return Matrix4::scaleXYZ(v.x, v.y, v.z);
}

Matrix4 Matrix4::translateXYZ(float x, float y, float z){
	return Matrix4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		x, y, z, 1.0f);
}

Matrix4 Matrix4::translateXYZ(const Vector3 & v){
	return Matrix4::translateXYZ(v.x, v.y, v.z);
}

Matrix4 Matrix4::multiply(const Matrix4 & m1, const Matrix4 & m2){
	Matrix4 result;

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

Matrix4 Matrix4::multiply(const Matrix4 & m2) const
{
	return multiply(*this, m2);
}

Vector3 Matrix4::transform(const Vector3 & v, const Matrix4 & m){
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
	Vector3 result = {
		( v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0] ) / w,
		( v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1] ) / w,
		( v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2] ) / w
	};
	return result;
}

Matrix4 Matrix4::transpose(const Matrix4 & m){
	Matrix4 result = {
		m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
		m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
		m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
		m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]
	};
	return result;
}

Matrix4 Matrix4::transpose() const {
	return Matrix4::transpose(*this);
}

Matrix4 Matrix4::inverse(const Matrix4 & m){

	float a0 = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
	float a1 = m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0];
	float a2 = m.m[0][0] * m.m[1][3] - m.m[0][3] * m.m[1][0];
	float a3 = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
	float a4 = m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1];
	float a5 = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];
	float b0 = m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0];
	float b1 = m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0];
	float b2 = m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0];
	float b3 = m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1];
	float b4 = m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1];
	float b5 = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
	float det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;

	if(det == 0.0f){
		return m; // 逆行列が存在しない！
	}

	float invDet = 1.0f / det;
	Matrix4 result = {
		( m.m[1][1] * b5 - m.m[1][2] * b4 + m.m[1][3] * b3 ) * invDet,
		( -m.m[0][1] * b5 + m.m[0][2] * b4 - m.m[0][3] * b3 ) * invDet,
		( m.m[3][1] * a5 - m.m[3][2] * a4 + m.m[3][3] * a3 ) * invDet,
		( -m.m[2][1] * a5 + m.m[2][2] * a4 - m.m[2][3] * a3 ) * invDet,
		( -m.m[1][0] * b5 + m.m[1][2] * b2 - m.m[1][3] * b1 ) * invDet,
		( m.m[0][0] * b5 - m.m[0][2] * b2 + m.m[0][3] * b1 ) * invDet,
		( -m.m[3][0] * a5 + m.m[3][2] * a2 - m.m[3][3] * a1 ) * invDet,
		( m.m[2][0] * a5 - m.m[2][2] * a2 + m.m[2][3] * a1 ) * invDet,
		( m.m[1][0] * b4 - m.m[1][1] * b2 + m.m[1][3] * b0 ) * invDet,
		( -m.m[0][0] * b4 + m.m[0][1] * b2 - m.m[0][3] * b0 ) * invDet,
		( m.m[3][0] * a4 - m.m[3][1] * a2 + m.m[3][3] * a0 ) * invDet,
		( -m.m[2][0] * a4 + m.m[2][1] * a2 - m.m[2][3] * a0 ) * invDet,
		( -m.m[1][0] * b3 + m.m[1][1] * b1 - m.m[1][2] * b0 ) * invDet,
		( m.m[0][0] * b3 - m.m[0][1] * b1 + m.m[0][2] * b0 ) * invDet,
		( -m.m[3][0] * a3 + m.m[3][1] * a1 - m.m[3][2] * a0 ) * invDet,
		( m.m[2][0] * a3 - m.m[2][1] * a1 + m.m[2][2] * a0 ) * invDet
	};

	return result;
}

Matrix4 Matrix4::perspectiveFovLH(float FovAngleY, float AspectHByW, float NearZ, float FarZ){
	float    SinFov;
	float    CosFov;
	scalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

	float Height = CosFov / SinFov;
	float Width = Height / AspectHByW;
	float fRange = FarZ / ( FarZ - NearZ );

	Matrix4 M;
	M.m[0][0] = Width;
	M.m[0][1] = 0.0f;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = 0.0f;
	M.m[1][1] = Height;
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = 0.0f;
	M.m[2][1] = 0.0f;
	M.m[2][2] = fRange;
	M.m[2][3] = 1.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = -fRange * NearZ;
	M.m[3][3] = 0.0f;
	return M;
}

Matrix4 Matrix4::orthographicProjectionLH(float width, float height, float nearZ, float farZ, float farOffset) {
	return Matrix4(
		2.0f / width, 0, 0, 0,
		0, 2.0f / height, 0, 0,
		0, 0, 1.0f / (farZ - nearZ), 0,
		0, 0, nearZ / (nearZ - farZ) + farOffset, 1);
}

Vector3 Matrix4::positionFromWorld() const {
	return Vector3(m[3][0], m[3][1], m[3][2]);
}

Vector3 Matrix4::scaleFromWorld() const
{
	Vector3 result;
	result.x = Vector3::length({ m[0][0] ,m[0][1] ,m[0][2] });
	result.y = Vector3::length({ m[1][0] ,m[1][1] ,m[1][2] });
	result.z = Vector3::length({ m[2][0] ,m[2][1] ,m[2][2] });
	return result;
}

Quaternion Matrix4::rotation() const {

	Quaternion result;

	// 最大成分を検索
	float elem[4]; // 0:x, 1:y, 2:z, 3:w
	elem[0] = m[0][0] - m[1][1] - m[2][2] + 1.0f;
	elem[1] = -m[0][0] + m[1][1] - m[2][2] + 1.0f;
	elem[2] = -m[0][0] - m[1][1] + m[2][2] + 1.0f;
	elem[3] = m[0][0] + m[1][1] + m[2][2] + 1.0f;

	unsigned biggestIndex = 0;
	for (int i = 1; i < 4; i++) {
		if (elem[i] > elem[biggestIndex])
			biggestIndex = i;
	}

	if (elem[biggestIndex] < 0.0f) {
		return result; // 引数の行列に間違いあり！
	}

	// 最大要素の値を算出
	float *q[4] = { &result.x, &result.y, &result.z, &result.w };
	float v = sqrtf(elem[biggestIndex]) * 0.5f;
	*q[biggestIndex] = v;
	float mult = 0.25f / v;

	switch (biggestIndex) {
	case 0: // x
		result.y = (m[0][1] + m[1][0]) * mult;
		result.z = (m[2][0] + m[0][2]) * mult;
		result.w = (m[1][2] - m[2][1]) * mult;
		break;
	case 1: // y
		result.x = (m[0][1] + m[1][0]) * mult;
		result.z = (m[1][2] + m[2][1]) * mult;
		result.w = (m[2][0] - m[0][2]) * mult;
		break;
	case 2: // z
		result.x = (m[2][0] + m[0][2]) * mult;
		result.y = (m[1][2] + m[2][1]) * mult;
		result.w = (m[0][1] - m[1][0]) * mult;
		break;
	case 3: // w
		result.x = (m[1][2] - m[2][1]) * mult;
		result.y = (m[2][0] - m[0][2]) * mult;
		result.z = (m[0][1] - m[1][0]) * mult;
		break;
	}

	return result;
}

Vector3 Matrix4::translate() const{
	return Vector3(m[3][0], m[3][1], m[3][2]);
}

Vector3 Matrix4::scale() const{
	Vector3 result;
	result.x = Vector3::length(Vector3(m[0][0], m[0][1], m[0][2]));
	result.y = Vector3::length(Vector3(m[1][0], m[1][1], m[1][2]));
	result.z = Vector3::length(Vector3(m[2][0], m[2][1], m[2][2]));
	return result;
}

Matrix4 Matrix4::inverse() const {
	return Matrix4::inverse(*this);
}

Matrix4 Matrix4::matrixFromQuaternion(const Quaternion& q){
	Matrix4 result = Matrix4::identity;

	const float x = q.x;
	const float y = q.y;
	const float z = q.z;
	const float w = q.w;

	result.m[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	result.m[0][1] = 2.0f * x * y + 2.0f * w * z;
	result.m[0][2] = 2.0f * x * z - 2.0f * w * y;

	result.m[1][0] = 2.0f * x * y - 2.0f * w * z;
	result.m[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	result.m[1][2] = 2.0f * y * z + 2.0f * w * x;

	result.m[2][0] = 2.0f * x * z + 2.0f * w * y;
	result.m[2][1] = 2.0f * y * z - 2.0f * w * x;
	result.m[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;

	return result;
}

bool Matrix4::operator==(const Matrix4 & m){
	return mv[0] == m.mv[0] && mv[1] == m.mv[1] && mv[2] == m.mv[2] && mv[3] == m.mv[3];
}

Matrix4 Matrix4::createWorldMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale) {
	Matrix4 result = Matrix4::scaleXYZ(scale);
	result = Matrix4::multiply(result, Matrix4::matrixFromQuaternion(rotation));
	result = Matrix4::multiply(result, Matrix4::translateXYZ(position));
	return result;
}

inline void scalarSinCos(float * pSin, float * pCos, float Value){

	assert(pSin);
	assert(pCos);

	// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = M_1DIV2PI*Value;
	if(Value >= 0.0f){
		quotient = (float)( (int)( quotient + 0.5f ) );
	}
	else{
		quotient = (float)( (int)( quotient - 0.5f ) );
	}
	float y = Value - M_2PI*quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
	float sign;
	if(y > M_PIDIV2){
		y = M_PI - y;
		sign = -1.0f;
	}
	else if(y < -M_PIDIV2){
		y = -M_PI- y;
		sign = -1.0f;
	}
	else{
		sign = +1.0f;
	}

	float y2 = y * y;

	// 11-degree minimax approximation
	*pSin = ( ( ( ( ( -2.3889859e-08f * y2 + 2.7525562e-06f ) * y2 - 0.00019840874f ) * y2 + 0.0083333310f ) * y2 - 0.16666667f ) * y2 + 1.0f ) * y;

	// 10-degree minimax approximation
	float p = ( ( ( ( -2.6051615e-07f * y2 + 2.4760495e-05f ) * y2 - 0.0013888378f ) * y2 + 0.041666638f ) * y2 - 0.5f ) * y2 + 1.0f;
	*pCos = sign*p;
}
