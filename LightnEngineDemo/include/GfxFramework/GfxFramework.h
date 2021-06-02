#pragma once
#include <GfxCore/GfxModuleSettings.h>

class LTN_GFX_FRAMEWORK_API GfxFramework {
public:
	virtual void initialize() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	virtual void terminate() = 0;

	static GfxFramework* Get();
};