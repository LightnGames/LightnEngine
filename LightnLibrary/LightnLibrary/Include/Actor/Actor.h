#pragma once

#include <Task/GameTask.h>
#include <LMath.h>

class Actor :public GameTask {

public:

	Actor();

	void setUpTask() override;

	void start() override;

	void update(float deltaTime) override;

	//コンポーネントを追加する
	template<typename T>
	RefPtr<T> addComponent() {
		auto child = makeChild<T>();
		child->attachToActor(this);

		return child;
	}

	void setActorPosition(const Vector3& position);
	void setActorRotation(const Quaternion& rotation);
	void setActorScale(const Vector3& scale);
	void setActorMatrix(const Matrix4& matrix);

	Vector3 getActorPosition() const;
	Quaternion getActorRotation() const;
	Vector3 getActorScale() const;
	Matrix4 getActorMatrix() const;

private:

	TransformQ _transform;

};