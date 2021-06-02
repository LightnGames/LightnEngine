#pragma once
#include <Core/Type.h>
#include <Core/CoreModuleSettings.h>

constexpr f32 EPSILON = 0.000001f;
constexpr f32 PI = 3.141592654f;
constexpr f32 PI2 = 6.283185307f;
constexpr f32 DIVPI = 0.318309886f;
constexpr f32 DIV2PI = 0.159154943f;
constexpr f32 PIDIV2 = 1.570796327f;
constexpr f32 PIDIV4 = 0.785398163f;

constexpr f32 RadToDeg(f32 radian) {
	return radian * (180.0f / PI);
}

constexpr f32 DegToRad(f32 degree) {
	return degree * (PI / 180.0f);
}

struct Float2 {
	f32 _x = 0;
	f32 _y = 0;
};

struct Float3 {
	Float3() :_x(0.0f), _y(0.0f), _z(0.0f) {}
	Float3(f32 x, f32 y, f32 z) :_x(x), _y(y), _z(z) {}
	f32 _x = 0;
	f32 _y = 0;
	f32 _z = 0;
};

struct Float4 {
	Float4() :_x(0.0f), _y(0.0f), _z(0.0f), _w(0.0f) {}
	Float4(f32 x, f32 y, f32 z, f32 w) :_x(x), _y(y), _z(z), _w(w) {}
	f32 _x = 0;
	f32 _y = 0;
	f32 _z = 0;
	f32 _w = 0;
};

struct Color3 {
	f32 _r = 0;
	f32 _g = 0;
	f32 _b = 0;
};

struct LTN_CORE_API Color4 {
	static const Color4 WHITE;
	static const Color4 DEEP_RED;
	static const Color4 RED;
	static const Color4 GREEN;
	static const Color4 DEEP_GREEN;
	static const Color4 YELLOW;
	static const Color4 BLUE;
	static const Color4 DEEP_BLUE;
	static const Color4 PURPLE;
	static const Color4 BLACK;

	f32 _r = 0;
	f32 _g = 0;
	f32 _b = 0;
	f32 _a = 0;
};

// 2�����x�N�g���N���X
struct LTN_CORE_API Vector2 {
	Vector2() :_x(0.0f), _y(0.0f){}
	Vector2(const f32 x, const f32 y) :_x(x), _y(y){}

	f32 _x;
	f32 _y;
};

LTN_CORE_API Vector2 operator + (const Vector2& v);
LTN_CORE_API Vector2 operator - (const Vector2& v);

LTN_CORE_API Vector2& operator += (Vector2& v1, const Vector2& v2);
LTN_CORE_API Vector2& operator -= (Vector2& v1, const Vector2& v2);
LTN_CORE_API Vector2& operator *= (Vector2& v, f32 s);
LTN_CORE_API Vector2& operator /= (Vector2& v, f32 s);

LTN_CORE_API Vector2 operator + (const Vector2& v1, const Vector2& v2);
LTN_CORE_API Vector2 operator - (const Vector2& v1, const Vector2& v2);
LTN_CORE_API Vector2 operator * (const Vector2& v, f32 s);
LTN_CORE_API Vector2 operator * (f32 s, const Vector2& v);
LTN_CORE_API Vector2 operator / (const Vector2& v, f32 s);

// 3�����x�N�g���N���X
struct LTN_CORE_API Vector3 {
	Vector3() :_x(0.0f), _y(0.0f), _z(0.0f){}
	Vector3(const f32 x, const f32 y, const f32 z) :_x(x), _y(y), _z(z){}
	Vector3(const f32 xyz) :_x(xyz), _y(xyz), _z(xyz){}
	Vector3(Float3 vector) :_x(vector._x), _y(vector._y), _z(vector._z) {}

	//�O��
	static Vector3 cross(const Vector3& v1, const Vector3& v2);

	//����
	static f32 dot(const Vector3& v1, const Vector3& v2);

	//����
	static f32 length(const Vector3& v);
	f32 getLength() const;

	//��_�Ԃ̋���
	static f32 distance(const Vector3& v1, const Vector3& v2);

	//�P�ʃx�N�g����
	static Vector3 normalize(const Vector3& v);
	Vector3 getNormalize() const;

	//��̊p�x�����v�Z
	static f32 angle(const Vector3& v1, const Vector3& v2);

	//��̃x�N�g���̌�������������
	static f32 equalRotator(const Vector3& v1, const Vector3& v2);

	//���`�⊮
	static Vector3 lerp(const Vector3& start, const Vector3& end, f32 t);

	static Vector3 Min(const Vector3& v1, const Vector3& v2);
	static Vector3 Max(const Vector3& v1, const Vector3& v2);

	Float3 getFloat3() const;

	//�萔
	static const Vector3 Up;
	static const Vector3 Right;
	static const Vector3 Forward;
	static const Vector3 Zero;
	static const Vector3 One;

	f32 _x = 0.0f;
	f32 _y = 0.0f;
	f32 _z = 0.0f;
};

LTN_CORE_API Vector3 operator + (const Vector3& v);
LTN_CORE_API Vector3 operator - (const Vector3& v);

LTN_CORE_API Vector3& operator += (Vector3& v1, const Vector3& v2);
LTN_CORE_API Vector3& operator -= (Vector3& v1, const Vector3& v2);
LTN_CORE_API Vector3& operator *= (Vector3& v, f32 s);
LTN_CORE_API Vector3& operator /= (Vector3& v, f32 s);

LTN_CORE_API Vector3 operator + (const Vector3& v1, const Vector3& v2);
LTN_CORE_API Vector3 operator - (const Vector3& v1, const Vector3& v2);
LTN_CORE_API Vector3 operator * (const Vector3& v, f32 s);
LTN_CORE_API Vector3 operator * (f32 s, const Vector3& v);
LTN_CORE_API Vector3 operator / (const Vector3& v, f32 s);

struct LTN_CORE_API Vector4 {
	Vector4() :x(0.0f), y(0.0f), z(0.0f), w(0.0f){}

	Vector4(const f32 x, const f32 y, const f32 z, const f32 w) :x(x), y(y), z(z), w(w){}
	Vector4(const Vector3& v, const f32 w = 0.0f) :x(v._x), y(v._y), z(v._z), w(w){}

	Vector3 toVector3() const {
		return Vector3(x, y, z);
	}

	bool operator == (const Vector4& v) { return x == v.x && y == v.y && z == v.z && w == v.w; }

	f32 x;
	f32 y;
	f32 z;
	f32 w;
};

// �P�����Z�q�I�[�o�[���[�h
Vector4 operator + (const Vector4& v);
Vector4 operator - (const Vector4& v);
Vector4& operator *= (Vector4& v, f32 s);

struct LTN_CORE_API Quaternion {
	Quaternion() :x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Quaternion(f32 x, f32 y, f32 z, f32 w) :x(x), y(y), z(z), w(w) {}
	Quaternion(const Vector3& axis, f32 angle);

	//�N�H�[�^�j�I�����m�̓���
	static f32 dot(const Quaternion& q1, const Quaternion& q2);

	//�N�H�[�^�j�I�������ʐ��`�⊮����
	static Quaternion slerp(const Quaternion q1, const Quaternion& q2, f32 t, bool nearRoute = true);

	//�I�C���[�p����N�H�[�^�j�I���𐶐�
	static Quaternion euler(const Vector3& euler, bool valueIsRadian = false);
	static Quaternion euler(f32 pitch, f32 yaw, f32 roll, bool valueIsRadian = false);

	//�x�N�g�����N�H�[�^�j�I���ŉ�]
	static Vector3 rotVector(const Quaternion& q, const Vector3& v);

	//direction��������]���쐬
	static Quaternion lookRotation(const Vector3& direction, const Vector3& up = Vector3::Up);

	//�t�N�H�[�^�j�I��
	Quaternion inverse() const;

	//�I�C���[�p�x���擾
	Vector3 toEulerAngle() const;

	//�߂�l�̓��W�A��
	f32 getRoll(bool reprojectAxis = true) const;
	f32 getPitch(bool reprojectAxis = true) const;
	f32 getYaw(bool reprojectAxis = true) const;

	static const Quaternion Identity;

	f32 x, y, z, w;
};

Quaternion operator - (const Quaternion& q);
Quaternion& operator += (Quaternion& q1, const Quaternion& q2);
Quaternion& operator -= (Quaternion& q1, const Quaternion& q2);
Quaternion& operator *= (Quaternion& q1, const Quaternion& q2);
Quaternion& operator *= (Quaternion& q1, f32 s);
Quaternion& operator /= (Quaternion& q, f32 s);
Quaternion operator + (const Quaternion& q1, const Quaternion& q2);
Quaternion operator - (const Quaternion& q1, const Quaternion& q2);
Quaternion operator * (const Quaternion& q1, const Quaternion& q2);
Quaternion operator * (const Quaternion& q, f32 s);
Quaternion operator * (f32 s, const Quaternion& q);
Quaternion operator / (const Quaternion& q, f32 s);

struct LTN_CORE_API Matrix34 {
	Matrix34() :m{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f} } {}
	Matrix34(
		f32 m11, f32 m12, f32 m13,
		f32 m21, f32 m22, f32 m23,
		f32 m31, f32 m32, f32 m33,
		f32 m41, f32 m42, f32 m43) :m{
			{ m11, m12, m13},
			{ m21, m22, m23},
			{ m31, m32, m33},
			{ m41, m42, m43} } {}

	union {
		f32 m[4][3];
		Vector3 mv[4];
	};
};

struct LTN_CORE_API Matrix43 {
	Matrix43() :m{
		{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f}} {
	}
	Matrix43(
		f32 m11, f32 m12, f32 m13, f32 m14,
		f32 m21, f32 m22, f32 m23, f32 m24,
		f32 m31, f32 m32, f32 m33, f32 m34) :m{
			{ m11, m12, m13, m14},
			{ m21, m22, m23, m24},
			{ m31, m32, m33, m34}} {
	}

	union {
		f32 m[3][4];
		Vector4 mv[3];
	};
};

struct LTN_CORE_API Matrix4 {
	Matrix4() :m{
			{0.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 0.0f}} {}
	Matrix4(
		f32 m11, f32 m12, f32 m13, f32 m14,
		f32 m21, f32 m22, f32 m23, f32 m24,
		f32 m31, f32 m32, f32 m33, f32 m34,
		f32 m41, f32 m42, f32 m43, f32 m44) :m{
			{ m11, m12, m13, m14},
			{ m21, m22, m23, m24},
			{ m31, m32, m33, m34},
			{ m41, m42, m43, m44 } } {}

	//��]�l�����]�s����쐬
	static Matrix4 rotateX(f32 pitch);
	static Matrix4 rotateY(f32 yaw);
	static Matrix4 rotateZ(f32 roll);
	static Matrix4 rotate(f32 pitch, f32 yaw, f32 roll);
	static Matrix4 rotate(const Vector3& eulerAngle);
	static Matrix4 rotateAxis(const Vector3& axis);

	//XYZ����g��k���s��𐶐�
	static Matrix4 scale(f32 x, f32 y, f32 z);
	static Matrix4 scale(const Vector3& scale);

	//XYZ���畽�s�ړ��s��𐶐�
	static Matrix4 translate(f32 x, f32 y, f32 z);
	static Matrix4 translate(const Vector3& position);

	//�s��̏�Z
	static Matrix4 multiply(const Matrix4& m1, const Matrix4& m2);

	//�x�N�g���ƍs��̏�Z
	static Vector3 transform(const Vector3& v, const Matrix4& m);
	static Vector3 transformNormal(const Vector3& v, const Matrix4& m);

	//�]�u�s��
	static Matrix4 transpose(const Matrix4& m);
	Matrix4 transpose() const;

	//�t�s��
	static Matrix4 inverse(const Matrix4& m);

	//������W�n�ˉe�ϊ��s��𐶐�
	static Matrix4 perspectiveFovLH(f32 FovAngleY, f32 AspectHByW, f32 NearZ, f32 FarZ);

	//���蕽�t���e�ˉe�Ԋҍs��𐶐�
	static Matrix4 orthographicProjectionLH(f32 width, f32 height, f32 nearZ, f32 farZ, f32 farOffset = 0.0f);

	//���s�ړ��s����擾
	Vector3 getTranslate() const;

	//�N�H�[�^�j�I�����擾
	Quaternion getRotation() const;

	//�g��k���l���擾
	Vector3 getScale() const;

	//�t�s��
	Matrix4 inverse() const;

	//���[���h�s�񂩂烏�[���h���W���擾
	Vector3 positionFromWorld() const;
	Vector3 scaleFromWorld() const;
	Matrix43 getMatrix43() const;

	//�N�H�[�^�j�I�������]�s��𐶐�
	static Matrix4 matrixFromQuaternion(const Quaternion& q);

	//�ʒu�A��]�A�傫�����烏�[���h�s����쐬
	static Matrix4 createWorldMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale);

	//�P�ʍs��
	static const Matrix4 Identity;

	bool operator == (const Matrix4& m);

	Matrix4 operator *(const Matrix4& m1) const {
		return Matrix4::multiply(*this, m1);
	}

	//Matrix4 operator *(const Vector4& m1) const {
	//	return Matrix4::multiply(*this, m1);
	//}


	union {
		f32 m[4][4];
		Vector4 mv[4];
	};
};