#include "LightComponent.h"



LightComponent::LightComponent() {
}


LightComponent::~LightComponent() {
}

void LightComponent::draw() {
}

void LightComponent::setIntensity(float intensity) {
	_intensity = intensity;
}

float LightComponent::getIntensity() const {
	return _intensity;
}
