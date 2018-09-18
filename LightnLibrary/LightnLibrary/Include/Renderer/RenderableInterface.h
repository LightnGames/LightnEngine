#pragma once

struct DrawSettings;

class RenderableEntity {

public:

	virtual void draw(const DrawSettings& drawSettings) = 0;

	virtual void drawDepth(const DrawSettings& drawSettings) = 0;
};

class RenderableObject {

public:
	RenderableObject() {}

	virtual ~RenderableObject() {}

};

class LightEntity {

public:
	virtual void draw(const DrawSettings& drawSettings) = 0;
};