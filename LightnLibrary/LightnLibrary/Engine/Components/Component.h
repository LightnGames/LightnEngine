#pragma once

#include <Util/RefPtr.h>
#include <Task/GameTask.h>
#include <LMath.h>

class Actor;

class Component :public GameTask{

public:

	Component();

	virtual ~Component();

	void setUp();

	void attachToActor(RefPtr<Actor> parentActor);

	virtual void update(float deltaTime) override;

	void setLocalPosition(const Vector3& position);
	void setLocalRotation(const Quaternion& rotation);

	void setWorldPosition(const Vector3& position);
	void setWorldRotation(const Quaternion& rotation);
	void setLocalScale(const Vector3& scale);
	void setWorldMatrix(const Matrix4& matrix);

	Vector3 forwardVector() const;
	Vector3 rightVector() const;
	Vector3 upVector() const;

	Vector3 getWorldPosition() const;
	Quaternion getWorldRotation() const;
	Quaternion getLocalRotation() const;
	Vector3 getLocalScale() const;
	Matrix4 getWorldMatrix() const;

protected:

	TransformQ _localTransform;
	RefPtr<Actor> _parentActor;
};