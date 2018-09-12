#pragma once

#include <LMath.h>
#include <Component/Component.h>

class Light;
struct ShadowResource;
class LightComponent :public Component {

public:

	LightComponent();
	~LightComponent();

	void draw();

	void setIntensity(float intensity);

	void setShadowSize(uint32 size);

	float getIntensity() const;

	uint32 getShadowSize() const;

	bool isEnableShadow() const;

	void enableShadow(bool enable);

	RefPtr<ShadowResource> getShadowResource();

protected:

	RefPtr<Light> _light;
	std::unique_ptr<ShadowResource> _shadow;
	uint32 _shadowSize;

private:

	float _intensity;

};

