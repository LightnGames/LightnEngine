#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include "Quaternion.h"

union Matrix4{

	Matrix4() :
	m{ 
		{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f}
	}{}

	Matrix4(float m11, float m12, float m13, float m14,
			float m21, float m22, float m23, float m24,
			float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44) :m{
			{ m11, m12, m13, m14},
			{ m21, m22, m23, m24},
			{ m31, m32, m33, m34},
			{ m41, m42, m43, m44 }}{
	}

	//回転値から回転行列を作成
	static Matrix4 rotateX(float pitch);
	static Matrix4 rotateY(float yaw);
	static Matrix4 rotateZ(float roll);
	static Matrix4 rotateXYZ(float pitch, float yaw, float roll);
	static Matrix4 rotateAxis(const Vector3& axis);

	//XYZから拡大縮小行列を生成
	static Matrix4 scaleXYZ(float x, float y, float z);
	static Matrix4 scaleXYZ(const Vector3& v);

	//XYZから平行移動行列を生成
	static Matrix4 translateXYZ(float x, float y, float z);
	static Matrix4 translateXYZ(const Vector3& v);

	//行列の乗算
	static Matrix4 multiply(const Matrix4& m1, const Matrix4& m2);
	Matrix4 multiply(const Matrix4& m2) const;

	//ベクトルと行列の乗算
	static Vector3 transform(const Vector3& v, const Matrix4& m);

	//転置行列
	static Matrix4 transpose(const Matrix4& m);
	Matrix4 transpose() const;

	//逆行列
	static Matrix4 inverse(const Matrix4& m);

	//左手座標系射影変換行列を生成
	static Matrix4 perspectiveFovLH(float FovAngleY, float AspectHByW, float NearZ, float FarZ);

	//平行移動行列を取得
	Vector3 translate() const;

	//クォータニオンを取得
	Quaternion rotation() const;

	//拡大縮小値を取得
	Vector3 scale() const;

	//逆行列
	Matrix4 inverse() const;

	//ワールド行列からワールド座標を取得
	Vector3 positionFromWorld() const;
	Vector3 scaleFromWorld() const;

	//クォータニオンから回転行列を生成
	static Matrix4 matrixFromQuaternion(const Quaternion& q);

	//位置、回転、大きさからワールド行列を作成
	static Matrix4 createWorldMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale);

	//単位行列
	static const Matrix4 identity;

	bool operator == (const Matrix4& m);

	float m[4][4];

	Vector4 mv[4];
};

inline void scalarSinCos(float* pSin, float* pCos, float Value);