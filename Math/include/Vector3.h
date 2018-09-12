#pragma once

//3�����x�N�g���N���X
struct Vector3{

	Vector3() :x{ 0.0f }, y{ 0.0f }, z{ 0.0f }{
	}

	Vector3(const float x, const float y, const float z) :x{ x }, y{ y }, z{ z }{
	}

	Vector3(const float xyz) :x{ xyz }, y{ xyz }, z{ xyz }{
	}

	//�O��
	static Vector3 cross(const Vector3& v1, const Vector3& v2);

	//����
	static float dot(const Vector3& v1, const Vector3& v2);

	//����
	static float length(const Vector3& v);
	float length() const;

	//��_�Ԃ̋���
	static float distance(const Vector3& v1, const Vector3& v2);

	//�P�ʃx�N�g����
	static Vector3 normalize(const Vector3& v);
	Vector3 normalize() const;

	//��̊p�x�����v�Z
	static float Angle(const Vector3& v1, const Vector3& v2);

	//��̃x�N�g���̌�������������
	static float EqualRotator(const Vector3& v1, const Vector3& v2);

	//���`�⊮
	static Vector3 lerp(const Vector3& start, const Vector3& end, float t);

	//�萔
	static const Vector3 up;
	static const Vector3 right;
	static const Vector3 forward;
	static const Vector3 zero;
	static const Vector3 one;

	float x;
	float y;
	float z;
};

// �P�����Z�q�I�[�o�[���[�h
Vector3 operator + (const Vector3& v);
Vector3 operator - (const Vector3& v);

// ������Z�q�I�[�o�[���[�h
Vector3& operator += (Vector3& v1, const Vector3& v2);
Vector3& operator -= (Vector3& v1, const Vector3& v2);
Vector3& operator *= (Vector3& v, float s);
Vector3& operator /= (Vector3& v, float s);

// �Q�����Z�q�I�[�o�[���[�h
Vector3 operator + (const Vector3& v1, const Vector3& v2);
Vector3 operator - (const Vector3& v1, const Vector3& v2);
Vector3 operator * (const Vector3& v, float s);
Vector3 operator * (float s, const Vector3& v);
Vector3 operator / (const Vector3& v, float s);
