#include <Scene/SceneManager.h>
#include <Scene/Scene.h>

#include <Actor/Actor.h>
#include <Actor/StaticMeshActor.h>

#include <vector>

template<> SceneManager* Singleton<SceneManager>::mSingleton = 0;

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
}

void SceneManager::initialize() {
	_gameTaskManager = std::make_unique<GameTaskManager>();
}

void SceneManager::updateScene(float deltaTime) {
	_gameTaskManager->updateTasks(deltaTime);
}

void SceneManager::setupScene() {
	std::vector<Actor> actors;
}
