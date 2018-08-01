#include "Actor.h"
#include <Components/Component.h>

Actor::Actor() {
}

void Actor::setUpTask() {
}

void Actor::start() {
}

void Actor::update(float deltaTime) {
	GameTask::update(deltaTime);
}

void Actor::setActorPosition(const Vector3 & position) {
	_transform.position = position;
}

void Actor::setActorRotation(const Quaternion & rotation) {
	_transform.rotation = rotation;
}

void Actor::setActorScale(const Vector3 & scale) {
	_transform.scale = scale;
}

void Actor::setActorMatrix(const Matrix4 & matrix) {
	_transform.position = matrix.translate();
	_transform.rotation = matrix.rotation();
	_transform.scale = matrix.scale();
}

Vector3 Actor::getActorPosition() const {
	return _transform.position;
}

Quaternion Actor::getActorRotation() const {
	return _transform.rotation;
}

Vector3 Actor::getActorScale() const {
	return _transform.scale;
}

Matrix4 Actor::getActorMatrix() const {
	return Matrix4::createWorldMatrix(_transform.position, _transform.rotation, _transform.scale);
}
