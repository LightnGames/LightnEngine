#pragma once 
#include <GfxFramework/GfxFramework.h>

class GfxFrameworkImpl :public GfxFramework {
public:
	virtual void initialize() override;
	virtual void update() override;
	virtual void render() override;
	virtual void terminate() override;
	void processDeletion();

private:
};