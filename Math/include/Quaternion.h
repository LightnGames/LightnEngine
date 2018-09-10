#pragma once

#include "Vector3.h"

struct Quaternion{

	Quaternion();

	Quaternion(float x, float y, float z, float w);

	Quaternion(const Vector3& axis, float angle);

	//�N�H�[�^�j�I�����m�̓���
	static float dot(const Quaternion& q1, const Quaternion& q2);

	//�N�H�[�^�j�I�������ʐ��`�⊮����
	static Quaternion slerp(const Quaternion q1, const Quaternion& q2, float t, bool nearRoute = true);

	//�I�C���[�p����N�H�[�^�j�I���𐶐�
	static Quaternion euler(const Vector3& euler, bool valueIsRadian = false);

	//�x�N�g�����N�H�[�^�j�I���ŉ�]
	static Vector3 rotVector(const Quaternion& q, const Vector3& v);

	//direction��������]���쐬
	static Quaternion lookRotation(const Vector3& direction, const Vector3& up = Vector3::up);

	//�t�N�H�[�^�j�I��
	Quaternion inverse() const;

	//�I�C���[�p�x���擾
	Vector3 toEulerAngle() const;

	//�߂�l�̓��W�A��
	float getRoll(bool reprojectAxis = true) const;
	float getPitch(bool reprojectAxis = true) const;
	float getYaw(bool reprojectAxis = true) const;

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
