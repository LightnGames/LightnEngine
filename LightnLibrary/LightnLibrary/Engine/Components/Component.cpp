#include "Component.h"
#include "Actor/Actor.h"

Component::Component() :_parentActor{ nullptr } {
}

Component::~Component() {
}

void Component::setUp() {
}

void Component::attachToActor(RefPtr<Actor> parentActor) {
	_parentActor = parentActor;
}

void Component::update(float deltaTime) {
	GameTask::update(deltaTime);
}

void Component::setLocalPosition(const Vector3 & position){
	_localTransform.position = position;
}

void Component::setLocalRotation(const Quaternion & rotation){
	_localTransform.rotation = rotation;
}

void Component::addLocalRotation(const Quaternion & rotation) {
	_localTransform.rotation = _localTransform.rotation* rotation;
}

void Component::setWorldPosition(const Vector3 & position) {
	_localTransform.position = position - _parentActor->getActorPosition();
}

void Component::setWorldRotation(const Quaternion & rotation) {
	_localTransform.rotation = Matrix4::multiply(_parentActor->getActorMatrix().inverse(), Matrix4::matrixFromQuaternion(rotation)).rotation();
}

void Component::setLocalScale(const Vector3 & scale) {
	_localTransform.scale = scale;
}

void Component::setWorldMatrix(const Matrix4 & matrix) {
	setWorldPosition(matrix.translate());
	setWorldRotation(matrix.rotation());
	setLocalScale(matrix.scale());
}

Vector3 Component::forwardVector() const
{
	return Quaternion::rotVector(getWorldRotation(), Vector3::forward);
}

Vector3 Component::rightVector() const
{
	return Quaternion::rotVector(getWorldRotation(), Vector3::right);
}

Vector3 Component::upVector() const
{
	return Quaternion::rotVector(getWorldRotation(), Vector3::up);
}

Vector3 Component::getWorldPosition() const {
	return Quaternion::rotVector(_parentActor->getActorRotation(), _localTransform.position) + _parentActor->getActorPosition();
}

Vector3 Component::getLocalPosition() const {
	return _localTransform.position;
}

Quaternion Component::getWorldRotation() const {
	return _parentActor->getActorRotation()*_localTransform.rotation;
}

Quaternion Component::getLocalRotation() const {
	return _localTransform.rotation;
}

Vector3 Component::getLocalScale() const {
	return _localTransform.scale;
}

Matrix4 Component::getWorldMatrix() const {
	Matrix4 localMatrix = Matrix4::createWorldMatrix(_localTransform.position, _localTransform.rotation, _localTransform.scale);
	return Matrix4::multiply(localMatrix, _parentActor->getActorMatrix());
}

RefPtr<Actor> Component::parent() {
	return _parentActor;
}
