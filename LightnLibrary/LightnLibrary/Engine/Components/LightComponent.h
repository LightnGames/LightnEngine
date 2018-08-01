#pragma once

#include <LMath.h>
#include <Util/RefPtr.h>
#include <Components/Component.h>

class Light;
class LightComponent :public Component {

public:

	LightComponent();
	~LightComponent();

	void draw();

	void setIntensity(float intensity);

	float getIntensity() const;

protected:

	RefPtr<Light> _light;

private:

	float _intensity;

};

