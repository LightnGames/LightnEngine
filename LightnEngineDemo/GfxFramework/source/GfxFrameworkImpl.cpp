#include "GfxFrameworkImpl.h"
#include <DebugRenderer/ModuleSettings.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <DebugRenderer/impl/DebugRendererSystemImpl.h>
#include <MeshRenderer/MeshRendererSystem.h>
#include <GfxCore/impl/ViewSystemImpl.h>
#include <MaterialSystem/impl/MaterialSystemImpl.h>
#include <TextureSystem/impl/TextureSystemImpl.h>

GfxFrameworkImpl _gfxFramework;

void GfxFrameworkImpl::initialize() {
	GraphicsSystemImpl::Get()->initialize();
	DebugRendererSystemImpl::Get()->initialize();
	TextureSystemImpl::Get()->initialize();
	MaterialSystemImpl::Get()->initialize();
	MeshRendererSystem::Get()->initialize();

	GraphicsSystemImpl::Get()->setRenderPass([](CommandList* commandList) {
		ViewInfo* viewInfo = ViewSystemImpl::Get()->getView();
		DebugRendererSystemImpl::Get()->resetGpuCounter(commandList);
		if (ViewSystemImpl::Get()->isEnabledDebugFixedView()) {
			MeshRendererSystem::Get()->render(commandList, ViewSystemImpl::Get()->getDebugFixedView());
			MeshRendererSystem::Get()->renderDebugFixed(commandList, viewInfo);
		} else {
			MeshRendererSystem::Get()->render(commandList, viewInfo);
		}

		DebugRendererSystemImpl::Get()->render(commandList, viewInfo);
	});
}

void GfxFrameworkImpl::update() {
	if (!GraphicsSystemImpl::Get()->isInitialized()) {
		return;
	}

	GraphicsSystemImpl::Get()->beginDebugWindow();
	MeshRendererSystem::Get()->update();
	MaterialSystemImpl::Get()->update();
	TextureSystemImpl::Get()->update();
	DebugRendererSystemImpl::Get()->update();
	GraphicsSystemImpl::Get()->update();
	processDeletion();
}

void GfxFrameworkImpl::render() {
	if (!GraphicsSystemImpl::Get()->isInitialized()) {
		return;
	}

	GraphicsSystemImpl::Get()->render();
}

void GfxFrameworkImpl::terminate() {
	MeshRendererSystem::Get()->terminate();
	MaterialSystemImpl::Get()->terminate();
	TextureSystemImpl::Get()->terminate();
	DebugRendererSystemImpl::Get()->terminate();
	GraphicsSystemImpl::Get()->terminate();
}

void GfxFrameworkImpl::processDeletion() {
	MeshRendererSystem::Get()->processDeletion();
	MaterialSystemImpl::Get()->processDeletion();
	TextureSystemImpl::Get()->processDeletion();
	GraphicsSystemImpl::Get()->processDeletion();
}

GfxFramework* GfxFramework::Get() {
	return &_gfxFramework;
}
