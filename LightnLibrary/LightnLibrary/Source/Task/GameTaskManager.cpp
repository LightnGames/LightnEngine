#include <Task/GameTaskManager.h>
#include <Task/GameTask.h>

#include <Scene/Scene.h>

GameTaskManager::GameTaskManager() {
}

void GameTaskManager::initialize(const RefPtr<GameTask>& rootTask) {
	_sceneTask = rootTask;
}

void GameTaskManager::updateTasks(float deltaTime) {
	if (_sceneTask.get() != nullptr) {
		_sceneTask->update(deltaTime);
	}
}
