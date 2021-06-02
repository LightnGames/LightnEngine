#include <Core/Math.h>
#include <cmath>
#include <DirectXMath.h>

const Color4 Color4::WHITE{ 1.0f, 1.0f, 1.0f, 1.0f };
const Color4 Color4::RED{ 1.0f, 0.0f, 0.0f, 1.0f };
const Color4 Color4::DEEP_RED{ 0.5f, 0.0f, 0.0f, 1.0f };
const Color4 Color4::GREEN{ 0.0f, 1.0f, 0.0f, 1.0f };
const Color4 Color4::DEEP_GREEN{ 0.0f, 0.5f, 0.0f, 1.0f };
const Color4 Color4::YELLOW{ 1.0f, 1.0f, 0.0f, 1.0f };
const Color4 Color4::BLUE{ 0.0f, 0.0f, 1.0f, 1.0f };
const Color4 Color4::DEEP_BLUE{ 0.0f, 0.0f, 0.5f, 1.0f };
const Color4 Color4::PURPLE{ 1.0f, 0.0f, 1.0f, 1.0f };
const Color4 Color4::BLACK{ 0.0f, 0.0f, 0.0f, 1.0f };

const Vector3 Vector3::Up{ 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::Right{ 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Forward{ 0.0f, 0.0f, 1.0f };
const Vector3 Vector3::Zero{ 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::One{ 1.0f, 1.0f, 1.0f };

const Quaternion Quaternion::Identity{ 0.0f,0.0f,0.0f,1.0f };

const Matrix4 Matrix4::Identity{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

Vector2 operator+(const Vector2& v) {
	return v;
}

Vector2 operator-(const Vector2& v) {
	return Vector2(-v._x, -v._y);
}

Vector2& operator+=(Vector2& v1, const Vector2& v2) {
	v1._x += v2._x;
	v1._y += v2._y;
	return v1;
}

Vector2& operator-=(Vector2& v1, const Vector2& v2) {
	v1._x -= v2._x;
	v1._y -= v2._y;
	return v1;
}

Vector2& operator*=(Vector2& v, f32 s) {
	v._x *= s;
	v._y *= s;
	return v;
}

Vector2& operator/=(Vector2& v, f32 s) {
	v *= 1.0f / s;
	return v;
}

Vector2 operator+(const Vector2& v1, const Vector2& v2) {
	Vector2 result = v1;
	return result += v2;
}

Vector2 operator-(const Vector2& v1, const Vector2& v2) {
	Vector2 result = v1;
	return result -= v2;
}

Vector2 operator*(const Vector2& v, f32 s) {
	Vector2 result = v;
	return result *= s;
}

Vector2 operator*(f32 s, const Vector2& v) {
	Vector2 result = v;
	return result *= s;
}

Vector2 operator/(const Vector2& v, f32 s) {
	Vector2 result = v;
	return result /= s;
}

Vector3 Vector3::cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result(
		(v1._y * v2._z) - (v1._z * v2._y),
		(v1._z * v2._x) - (v1._x * v2._z),
		(v1._x * v2._y) - (v1._y * v2._x));
	return result;
}

f32 Vector3::dot(const Vector3& v1, const Vector3& v2) {
	return v1._x * v2._x + v1._y * v2._y + v1._z * v2._z;
}

f32 Vector3::length(const Vector3& v) {
	return static_cast<f32>(std::sqrt(dot(v, v)));
}

f32 Vector3::getLength() const {
	return Vector3::length(*this);
}

f32 Vector3::distance(const Vector3& v1, const Vector3& v2) {
	return length(v2 - v1);
}

Vector3 Vector3::normalize(const Vector3& v) {
	Vector3 result = v;
	float len = length(result);
	if (len != 0) {
		result /= len;
	}
	return result;
}

Vector3 Vector3::getNormalize() const {
	return Vector3::normalize(*this);
}

f32 Vector3::angle(const Vector3& v1, const Vector3& v2) {
	//float d = dot(normalize(v1), normalize(v2));
	//return Acos(Clamp(d, -1.0f, 1.0f));
	return 0;
}

f32 Vector3::equalRotator(const Vector3& v1, const Vector3& v2) {
	const Vector3 normV1 = normalize(v1);
	const Vector3 normV2 = normalize(v2);

	int result = 0;
	result += normV1._x == normV2._x;
	result += normV1._y == normV2._y;
	result += normV1._z == normV2._z;

	return result == 3;
}

Vector3 Vector3::lerp(const Vector3& start, const Vector3& end, f32 t) {
	//t = clamp(t, 0.0f, 1.0f);
	//return start * (1.0f - t) + end * t;
	return Vector3();
}

Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2) {
	return Vector3(
		v1._x < v2._x ? v1._x : v2._x,
		v1._y < v2._y ? v1._y : v2._y,
		v1._z < v2._z ? v1._z : v2._z);
}

Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2) {
	return Vector3(
		v1._x > v2._x ? v1._x : v2._x,
		v1._y > v2._y ? v1._y : v2._y,
		v1._z > v2._z ? v1._z : v2._z);
}

Float3 Vector3::getFloat3() const {
	return Float3(_x, _y, _z);
}

Vector3 operator+(const Vector3& v) {
	return v;
}

Vector3 operator-(const Vector3& v) {
	return Vector3(-v._x, -v._y, -v._z);
}

Vector3& operator+=(Vector3& v1, const Vector3& v2) {
	v1._x += v2._x;
	v1._y += v2._y;
	v1._z += v2._z;
	return v1;
}

Vector3& operator-=(Vector3& v1, const Vector3& v2) {
	v1._x -= v2._x;
	v1._y -= v2._y;
	v1._z -= v2._z;
	return v1;
}

Vector3& operator*=(Vector3& v, f32 s) {
	v._x *= s;
	v._y *= s;
	v._z *= s;
	return v;
}

Vector3& operator/=(Vector3& v, f32 s) {
	v *= 1.0f / s;
	return v;
}

Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 result = v1;
	return result += v2;
}

Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	Vector3 result = v1;
	return result -= v2;
}

Vector3 operator*(const Vector3& v, f32 s) {
	Vector3 result = v;
	return result *= s;
}

Vector3 operator*(f32 s, const Vector3& v) {
	Vector3 result = v;
	return result *= s;
}

Vector3 operator/(const Vector3& v, f32 s) {
	Vector3 result = v;
	return result /= s;
}

Vector4 operator+(const Vector4& v) {
	return Vector4();
}

Vector4 operator-(const Vector4& v) {
	return Vector4();
}

Vector4& operator*=(Vector4& v, f32 s) {
	return v;
}

Quaternion::Quaternion(const Vector3& axis, f32 angle) {
}

f32 Quaternion::dot(const Quaternion& q1, const Quaternion& q2) {
	return f32();
}

Quaternion Quaternion::slerp(const Quaternion q1, const Quaternion& q2, f32 t, bool nearRoute) {
	return Quaternion();
}

Quaternion Quaternion::euler(const Vector3& euler, bool valueIsRadian) {
	return Quaternion();
}

Quaternion Quaternion::euler(f32 pitch, f32 yaw, f32 roll, bool valueIsRadian) {
	return Quaternion();
}

Vector3 Quaternion::rotVector(const Quaternion& q, const Vector3& v) {
	return Vector3();
}

Quaternion Quaternion::lookRotation(const Vector3& direction, const Vector3& up) {
	return Quaternion();
}

Quaternion Quaternion::inverse() const {
	return Quaternion();
}

Vector3 Quaternion::toEulerAngle() const {
	return Vector3();
}

f32 Quaternion::getRoll(bool reprojectAxis) const {
	return f32();
}

f32 Quaternion::getPitch(bool reprojectAxis) const {
	return f32();
}

f32 Quaternion::getYaw(bool reprojectAxis) const {
	return f32();
}

Quaternion operator-(const Quaternion& q) {
	return Quaternion();
}

Quaternion& operator+=(Quaternion& q1, const Quaternion& q2) {
	return q1;
}

Quaternion& operator-=(Quaternion& q1, const Quaternion& q2) {
	return q1;
}

Quaternion& operator*=(Quaternion& q1, const Quaternion& q2) {
	return q1;
}

Quaternion& operator*=(Quaternion& q1, f32 s) {
	return q1;
}

Quaternion& operator/=(Quaternion& q, f32 s) {
	return q;
}

Quaternion operator+(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion();
}

Quaternion operator-(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion();
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion();
}

Quaternion operator*(const Quaternion& q, f32 s) {
	return Quaternion();
}

Quaternion operator*(f32 s, const Quaternion& q) {
	return Quaternion();
}

Quaternion operator/(const Quaternion& q, f32 s) {
	return Quaternion();
}

Matrix4 Matrix4::rotateX(f32 pitch) {
	f32 rad = pitch;
	f32 sin = std::sin(rad);
	f32 cos = std::cos(rad);
	Matrix4 result = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos, sin, 0.0f,
		0.0f, -sin, cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return result;
}

Matrix4 Matrix4::rotateY(f32 yaw) {
	f32 rad = yaw;
	f32 sin = std::sin(rad);
	f32 cos = std::cos(rad);
	Matrix4 result = {
		cos, 0.0f, -sin, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sin, 0.0f, cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return result;
}

Matrix4 Matrix4::rotateZ(f32 roll) {
	f32 rad = roll;
	f32 sin = std::sin(rad);
	f32 cos = std::cos(rad);
	Matrix4 result = {
		cos, sin, 0.0f, 0.0f,
		-sin, cos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return result;
}

Matrix4 Matrix4::rotate(f32 pitch, f32 yaw, f32 roll) {
	Matrix4 p = rotateX(pitch);
	Matrix4 y = rotateY(yaw);
	Matrix4 r = rotateZ(roll);
	return p * y * r;
}

Matrix4 Matrix4::rotate(const Vector3& eulerAngle) {
	return Matrix4::rotate(eulerAngle._x, eulerAngle._y, eulerAngle._z);
}

Matrix4 Matrix4::rotateAxis(const Vector3& axis) {
	return Matrix4();
}

Matrix4 Matrix4::scale(f32 x, f32 y, f32 z) {
	return Matrix4(
		x, 0.0f, 0.0f, 0.0f,
		0.0f, y, 0.0f, 0.0f,
		0.0f, 0.0f, z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::scale(const Vector3& scale) {
	return Matrix4::scale(scale._x, scale._y, scale._z);
}

Matrix4 Matrix4::translate(f32 x, f32 y, f32 z) {
	return Matrix4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		x, y, z, 1.0f);
}

Matrix4 Matrix4::translate(const Vector3& position) {
	return Matrix4::translate(position._x, position._y, position._z);
}

Matrix4 Matrix4::multiply(const Matrix4& m1, const Matrix4& m2) {
	Matrix4 result;
	for (s32 i = 0; i < 4; i++) {
		for (s32 j = 0; j < 4; j++) {
			for (s32 k = 0; k < 4; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}

	return result;
}

Vector3 Matrix4::transform(const Vector3& v, const Matrix4& m) {
	DirectX::XMVECTOR xv = DirectX::XMVectorSet(v._x, v._y, v._z, 1.0f);
	DirectX::XMMATRIX xm = DirectX::XMMatrixSet(
		m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
	DirectX::XMVECTOR resultV = DirectX::XMVector4Transform(xv, xm);
	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, resultV);
	return Vector3(result.x, result.y, result.z);
}

Vector3 Matrix4::transformNormal(const Vector3& v, const Matrix4& m) {
	DirectX::XMVECTOR xv = DirectX::XMVectorSet(v._x, v._y, v._z, 1.0f);
	DirectX::XMMATRIX xm = DirectX::XMMatrixSet(
		m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
	DirectX::XMVECTOR resultV = DirectX::XMVector3TransformNormal(xv, xm);
	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, resultV);
	return Vector3(result.x, result.y, result.z);
}

Matrix4 Matrix4::transpose(const Matrix4& m) {
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

Matrix4 Matrix4::inverse(const Matrix4& m) {
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
	float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

	if (det == 0.0f) {
		return m; // ‹ts—ñ‚ª‘¶Ý‚µ‚È‚¢I
	}

	float invDet = 1.0f / det;
	Matrix4 result = {
		(m.m[1][1] * b5 - m.m[1][2] * b4 + m.m[1][3] * b3) * invDet,
		(-m.m[0][1] * b5 + m.m[0][2] * b4 - m.m[0][3] * b3) * invDet,
		(m.m[3][1] * a5 - m.m[3][2] * a4 + m.m[3][3] * a3) * invDet,
		(-m.m[2][1] * a5 + m.m[2][2] * a4 - m.m[2][3] * a3) * invDet,
		(-m.m[1][0] * b5 + m.m[1][2] * b2 - m.m[1][3] * b1) * invDet,
		(m.m[0][0] * b5 - m.m[0][2] * b2 + m.m[0][3] * b1) * invDet,
		(-m.m[3][0] * a5 + m.m[3][2] * a2 - m.m[3][3] * a1) * invDet,
		(m.m[2][0] * a5 - m.m[2][2] * a2 + m.m[2][3] * a1) * invDet,
		(m.m[1][0] * b4 - m.m[1][1] * b2 + m.m[1][3] * b0) * invDet,
		(-m.m[0][0] * b4 + m.m[0][1] * b2 - m.m[0][3] * b0) * invDet,
		(m.m[3][0] * a4 - m.m[3][1] * a2 + m.m[3][3] * a0) * invDet,
		(-m.m[2][0] * a4 + m.m[2][1] * a2 - m.m[2][3] * a0) * invDet,
		(-m.m[1][0] * b3 + m.m[1][1] * b1 - m.m[1][2] * b0) * invDet,
		(m.m[0][0] * b3 - m.m[0][1] * b1 + m.m[0][2] * b0) * invDet,
		(-m.m[3][0] * a3 + m.m[3][1] * a1 - m.m[3][2] * a0) * invDet,
		(m.m[2][0] * a3 - m.m[2][1] * a1 + m.m[2][2] * a0) * invDet
	};

	return result;
}

Matrix4 Matrix4::perspectiveFovLH(f32 FovAngleY, f32 AspectHByW, f32 NearZ, f32 FarZ) {
	// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
	f32 value = 0.5f * FovAngleY;
	float quotient = DIV2PI * value;
	if (value >= 0.0f) {
		quotient = (float)((int)(quotient + 0.5f));
	}
	else {
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = value - PI2 * quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
	float sign;
	if (y > PIDIV2) {
		y = PI - y;
		sign = -1.0f;
	}
	else if (y < -PIDIV2) {
		y = -PI - y;
		sign = -1.0f;
	}
	else {
		sign = +1.0f;
	}

	float y2 = y * y;

	// 11-degree minimax approximation
	float    SinFov = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

	// 10-degree minimax approximation
	float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
	float    CosFov = sign * p;

	float height = CosFov / SinFov;
	float width = height / AspectHByW;
	float range = FarZ / (FarZ - NearZ);

	Matrix4 M;
	M.m[0][0] = width;
	M.m[0][1] = 0.0f;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = 0.0f;
	M.m[1][1] = height;
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = 0.0f;
	M.m[2][1] = 0.0f;
	M.m[2][2] = range;
	M.m[2][3] = 1.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = -range * NearZ;
	M.m[3][3] = 0.0f;
	return M;
}

Matrix4 Matrix4::orthographicProjectionLH(f32 width, f32 height, f32 nearZ, f32 farZ, f32 farOffset) {
	return Matrix4();
}

Vector3 Matrix4::getTranslate() const {
	return Vector3(m[3][0], m[3][1], m[3][2]);
}

Quaternion Matrix4::getRotation() const {
	return Quaternion();
}

Vector3 Matrix4::getScale() const {
	Vector3 result;
	result._x = Vector3(m[0][0], m[0][1], m[0][2]).getLength();
	result._y = Vector3(m[1][0], m[1][1], m[1][2]).getLength();
	result._z = Vector3(m[2][0], m[2][1], m[2][2]).getLength();
	return result;
}

Matrix4 Matrix4::inverse() const {
	return Matrix4::inverse(*this);
}

Vector3 Matrix4::positionFromWorld() const {
	return Vector3();
}

Vector3 Matrix4::scaleFromWorld() const {
	return Vector3();
}

Matrix43 Matrix4::getMatrix43() const {
	return Matrix43(m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1], m[2][2], m[2][3]);
}

Matrix4 Matrix4::matrixFromQuaternion(const Quaternion& q) {
	return Matrix4();
}

Matrix4 Matrix4::createWorldMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale) {
	return Matrix4();
}

bool Matrix4::operator==(const Matrix4& m) {
	return false;
}
