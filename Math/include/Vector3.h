#pragma once

//3次元ベクトルクラス
struct Vector3{

	Vector3() :x{ 0.0f }, y{ 0.0f }, z{ 0.0f }{
	}

	Vector3(const float x, const float y, const float z) :x{ x }, y{ y }, z{ z }{
	}

	Vector3(const float xyz) :x{ xyz }, y{ xyz }, z{ xyz }{
	}

	//外積
	static Vector3 cross(const Vector3& v1, const Vector3& v2);

	//内積
	static float dot(const Vector3& v1, const Vector3& v2);

	//長さ
	static float length(const Vector3& v);
	float length() const;

	//二点間の距離
	static float distance(const Vector3& v1, const Vector3& v2);

	//単位ベクトル化
	static Vector3 normalize(const Vector3& v);
	Vector3 normalize() const;

	//二つの角度差を計算
	static float Angle(const Vector3& v1, const Vector3& v2);

	//二つのベクトルの向きが等しいか
	static float EqualRotator(const Vector3& v1, const Vector3& v2);

	//線形補完
	static Vector3 lerp(const Vector3& start, const Vector3& end, float t);

	//定数
	static const Vector3 up;
	static const Vector3 right;
	static const Vector3 forward;
	static const Vector3 zero;
	static const Vector3 one;

	float x;
	float y;
	float z;
};

// 単項演算子オーバーロード
Vector3 operator + (const Vector3& v);
Vector3 operator - (const Vector3& v);

// 代入演算子オーバーロード
Vector3& operator += (Vector3& v1, const Vector3& v2);
Vector3& operator -= (Vector3& v1, const Vector3& v2);
Vector3& operator *= (Vector3& v, float s);
Vector3& operator /= (Vector3& v, float s);

// ２項演算子オーバーロード
Vector3 operator + (const Vector3& v1, const Vector3& v2);
Vector3 operator - (const Vector3& v1, const Vector3& v2);
Vector3 operator * (const Vector3& v, float s);
Vector3 operator * (float s, const Vector3& v);
Vector3 operator / (const Vector3& v, float s);
