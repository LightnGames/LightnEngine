#pragma once

struct DrawSettings;

class RenderableEntity {

public:

	virtual void draw(const DrawSettings& drawSettings) = 0;

	virtual void drawDepth(const DrawSettings& drawSettings) = 0;
};