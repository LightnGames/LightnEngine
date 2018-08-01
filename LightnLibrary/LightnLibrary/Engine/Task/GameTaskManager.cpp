#include "GameTaskManager.h"
#include "GameTask.h"

#include <Scene/Scene.h>

GameTaskManager::GameTaskManager() {
}

void GameTaskManager::initialize(const RefPtr<GameTask>& rootTask) {
	_sceneTask = rootTask;
}

void GameTaskManager::updateTasks(float deltaTime) {
	_sceneTask->update(deltaTime);
}
