#pragma once

#include <Task/GameTaskManager.h>
#include <Util/Singleton.h>
#include <memory>
#include <Scene/Scene.h>

class SceneManager :public Singleton<SceneManager>{

public:

	SceneManager();

	~SceneManager();

	void initialize();

	void updateScene(float deltaTime);

	template<class T>
	RefPtr<T> createScene() {
		_scene = std::make_unique<T>();
		_scene->start();
		_gameTaskManager->initialize(_scene.get());
		return (T*)_scene.get();
	}

private:

	void setupScene();

private:

	std::unique_ptr<Scene> _scene;
	std::unique_ptr<GameTaskManager> _gameTaskManager;
};