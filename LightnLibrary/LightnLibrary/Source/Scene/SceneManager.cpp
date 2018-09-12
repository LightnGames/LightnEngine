#include <Scene/SceneManager.h>
#include <Scene/Scene.h>
#include <Task/GameTaskManager.h>

#include <Actor/Actor.h>
#include <Actor/StaticMeshActor.h>

#include <vector>

template<> SceneManager* Singleton<SceneManager>::mSingleton = 0;

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
}

void SceneManager::initialize() {
	_scene = std::make_unique<Scene>();
	_gameTaskManager = std::make_unique<GameTaskManager>();
	_gameTaskManager->initialize(_scene.get());
}

void SceneManager::updateScene(float deltaTime) {
	_gameTaskManager->updateTasks(deltaTime);
}

void SceneManager::setupScene() {
	std::vector<Actor> actors;
}
