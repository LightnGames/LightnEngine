#pragma once
#include <DebugRenderer/ModuleSettings.h>
#include <Core/System.h>

class LTN_DEBUG_RENDERER_API DebugRendererSystem {
public:
	virtual void drawLine(Vector3 startPosition, Vector3 endPosition, Color4 color = Color4::RED) = 0;
	virtual void drawBox(Matrix4 matrix, Color4 color = Color4::RED) = 0;
	virtual void drawAabb(Vector3 boundsMin, Vector3 boundsMax, Color4 color = Color4::RED) = 0;
	virtual void drawFrustum(Matrix4 view, Matrix4 projection, Color4 color = Color4::RED) = 0;

	static DebugRendererSystem* Get();
};