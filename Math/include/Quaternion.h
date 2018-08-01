#pragma once

#include "Vector3.h"

struct Quaternion{

	Quaternion();

	Quaternion(float x, float y, float z, float w);

	Quaternion(const Vector3& axis, float angle);

	//クォータニオン同士の内積
	static float dot(const Quaternion& q1, const Quaternion& q2);

	//クォータニオンを球面線形補完する
	static Quaternion slerp(const Quaternion q1, const Quaternion& q2, float t);

	//オイラー角からクォータニオンを生成
	static Quaternion euler(const Vector3& euler);

	//ベクトルをクォータニオンで回転
	static Vector3 rotVector(const Quaternion& q, const Vector3& v);

	//逆クォータニオン
	Quaternion inverse() const;

	//オイラー角度を取得
	Vector3 toEulerAngle() const;

	static const Quaternion identity;

	float x, y, z, w;
};

Quaternion operator - (const Quaternion& q);

Quaternion& operator += (Quaternion& q1, const Quaternion& q2);

Quaternion& operator -= (Quaternion& q1, const Quaternion& q2);

Quaternion& operator *= (Quaternion& q1, const Quaternion& q2);

Quaternion& operator *= (Quaternion& q1, float s);

Quaternion& operator /= (Quaternion& q, float s);

Quaternion operator + (const Quaternion& q1, const Quaternion& q2);

Quaternion operator - (const Quaternion& q1, const Quaternion& q2);

Quaternion operator * (const Quaternion& q1, const Quaternion& q2);

Quaternion operator * (const Quaternion& q, float s);

Quaternion operator * (float s, const Quaternion& q);

Quaternion operator / (const Quaternion & q, float s);
