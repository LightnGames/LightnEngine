#pragma once

#include <Util/Singleton.h>
#include <memory>

class Scene;
class GameTaskManager;

class SceneManager :public Singleton<SceneManager>{

public:

	SceneManager();

	~SceneManager();

	void initialize();

	void updateScene(float deltaTime);

private:

	void setupScene();

private:

	std::unique_ptr<Scene> _scene;
	std::unique_ptr<GameTaskManager> _gameTaskManager;
};