#pragma once
#include <DirectXMath.h>
#include <cmath>
// from <Core/Math.h>

using uchar = unsigned char;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using float32 = float;
using float64 = double;
using long64 = long;
using llong64 = long long;
using ulong64 = unsigned long;
using ullong64 = unsigned long long;
//=======================================================================

constexpr inline ulong64 Hash(const char* str, uint32 length) {
	ulong64 hash = 5381;

	for (uint32 i = 0; i < length; ++i) {
		hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + c */
	}

	return hash;
}

inline bool Approximately(float a, float b) {
	return std::fabs(a - b) < FLT_EPSILON;
}

inline float Sqr(float value) {
	return std::sqrtf(value);
}

constexpr inline float DegToRad(float degree) {
	return DirectX::XMConvertToRadians(degree);
}


struct Float2 :public DirectX::XMFLOAT2 {
	Float2() :DirectX::XMFLOAT2(0.0f, 0.0f) {}
	Float2(float x, float y) :DirectX::XMFLOAT2(x, y) {}
	bool operator==(const Float2& left) const {
		return Approximately(x, left.x) && Approximately(y, left.y);
	}
};
struct Float3 :public DirectX::XMFLOAT3 {
	Float3() :DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) {}
	Float3(float x, float y, float z) :DirectX::XMFLOAT3(x, y, z) {}
	Float3(float value) :Float3(value, value, value) {}
	bool operator==(const Float3& left) const {
		return Approximately(x, left.x) && Approximately(y, left.y) && Approximately(z, left.z);
	}
};
struct Float4 :public DirectX::XMFLOAT4 {
	Float4() :DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) {}
	Float4(float x, float y, float z, float w) :DirectX::XMFLOAT4(x, y, z, w) {}
	Float4(float value) :Float4(value, value, value, value) {}
	bool operator==(const Float4& left) const {
		return Approximately(x, left.x) && Approximately(y, left.y) && Approximately(z, left.z) && Approximately(w, left.w);
	}
};

struct Color3 {
	constexpr Color3() :_r(0.0f), _g(0.0f), _b(0.0f) {}
	constexpr Color3(float r, float g, float b) : _r(r), _g(g), _b(b) {}
	constexpr Color3(float colorValue) : _r(colorValue), _g(colorValue), _b(colorValue) {}

	static constexpr Color3 Red() { return Color3(1.0f, 0.0f, 0.0f); }
	static constexpr Color3 Green() { return Color3(0.0f, 1.0f, 0.0f); }
	static constexpr Color3 Blue() { return Color3(0.0f, 0.0f, 1.0f); }

	float _r;
	float _g;
	float _b;
};

struct Color4 :public Color3 {
	constexpr Color4() :Color3(0.0f, 0.0f, 0.0f), _a(0.0f) {}
	constexpr Color4(float r, float g, float b, float a) : Color3(r, g, b), _a(a) {}
	constexpr Color4(float colorValue) : Color3(colorValue), _a(colorValue) {}
	constexpr Color4(const Color3& color, float a) : Color3(color), _a(a) {}

	static constexpr Color4 Red() { return Color4(Color3::Red(), 1.0f); }
	static constexpr Color4 Green() { return Color4(Color3::Green(), 1.0f); }
	static constexpr Color4 Blue() { return Color4(Color3::Blue(), 1.0f); }

	float _a;
};

struct Vector3 {
	Vector3() {}
	Vector3(const Float3& value) :_vector(DirectX::XMLoadFloat3(&value)) {}
	Vector3(const DirectX::XMVECTOR& value) :_vector(value) {}
	Vector3(float x, float y, float z) :Vector3(Float3(x, y, z)) {}
	Vector3(float value) :Vector3(Float3(value)) {}

	static Vector3 Right() { return Vector3(1.0f, 0.0f, 0.0f); }
	static Vector3 Up() { return Vector3(0.0f, 1.0f, 0.0f); }
	static Vector3 Forward() { return Vector3(0.0f, 0.0f, 1.0f); }

	static float GetDistance(const Vector3& p1, const Vector3& p2) {
		return (p1 - p2).getLength();
	}

	static float Dot(const Vector3& v1, const Vector3& v2) {
		return Vector3(DirectX::XMVector3Dot(v1._vector, v2._vector)).getX();
	}

	static Vector3 Cross(const Vector3& v1, const Vector3& v2) {
		return Vector3(DirectX::XMVector3Cross(v1._vector, v2._vector));
	}

	Vector3 operator -(const Vector3& v) const {
		return Vector3(DirectX::XMVectorSubtract(_vector, v._vector));
	}

	Vector3 operator +(const Vector3& v) const {
		return Vector3(DirectX::XMVectorAdd(_vector, v._vector));
	}

	Vector3 operator *(float value) const {
		return Vector3(DirectX::XMVectorScale(_vector, value));
	}

	Vector3 operator /(float value) const {
		return Vector3(DirectX::XMVectorDivide(_vector, DirectX::XMVectorReplicate(value)));
	}

	Float3 getFloat3() const {
		Float3 value;
		DirectX::XMStoreFloat3(&value, _vector);
		return value;
	}

	float getX() const {
		return DirectX::XMVectorGetX(_vector);
	}

	float getY() const {
		return DirectX::XMVectorGetY(_vector);
	}

	float getZ() const {
		return DirectX::XMVectorGetZ(_vector);
	}

	float getLength() const {
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(_vector));
	}

	float getLengthSqr() const {
		return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(_vector));
	}

	Vector3 getNormalize() const {
		return *this / getLength();
	}

	DirectX::XMVECTOR _vector;
};

struct Vector4 {
	DirectX::XMVECTOR _vector;
};

struct Matrix4 {
	Matrix4() :_matrix(DirectX::XMMatrixIdentity()) {}
	Matrix4(DirectX::XMMATRIX matrix) :_matrix(matrix) {}
	static Matrix4 identity() { return Matrix4(); }
	static Matrix4 perspectiveFovLH(float fov, float aspectRatio, float nearZ, float farZ) {
		return Matrix4(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ));
	}

	static Matrix4 translate(float x, float y, float z) {
		return Matrix4(DirectX::XMMatrixTranslation(x, y, z));
	}

	static Matrix4 translate(const Float3& translate) {
		return Matrix4::translate(translate.x, translate.y, translate.z);
	}

	static Matrix4 inverse(const Matrix4& matrix) {
		return Matrix4(DirectX::XMMatrixInverse(nullptr, matrix._matrix));
	}

	static Matrix4 transpose(const Matrix4& matrix) {
		return Matrix4(DirectX::XMMatrixTranspose(matrix._matrix));
	}

	Matrix4 inverse() const {
		return Matrix4::inverse(*this);
	}

	Matrix4 transpose() const {
		return Matrix4::transpose(*this);
	}

	Matrix4 operator *(const Matrix4& m1) const {
		return Matrix4(_matrix* m1._matrix);
	}

	DirectX::XMMATRIX _matrix;
};