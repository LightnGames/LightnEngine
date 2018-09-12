#include <Component/LightComponent.h>
#include <Renderer/Light/Light.h>


LightComponent::LightComponent(): _shadow{ nullptr } {
}


LightComponent::~LightComponent() {
}

void LightComponent::draw() {
}

void LightComponent::setIntensity(float intensity) {
	_intensity = intensity;
}

void LightComponent::setShadowSize(uint32 size) {
	_shadowSize = size;
}

float LightComponent::getIntensity() const {
	return _intensity;
}

uint32 LightComponent::getShadowSize() const {
	return _shadowSize;
}

bool LightComponent::isEnableShadow() const {
	return _shadow.get() != nullptr;
}

void LightComponent::enableShadow(bool enable){

	if ((_shadow.get() == nullptr) && enable) {
		_shadow = std::make_unique<ShadowResource>(_light->createShadow(_shadowSize));
		return;
	}

	if ((_shadow.get() != nullptr) && !enable) {
		_shadow = nullptr;
		return;
	}
}

RefPtr<ShadowResource> LightComponent::getShadowResource() {
	return _shadow.get();
}
