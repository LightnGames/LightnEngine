#pragma once

struct DrawSettings;

class LightEntity {

public:
	virtual void draw(const DrawSettings& drawSettings) = 0;
};