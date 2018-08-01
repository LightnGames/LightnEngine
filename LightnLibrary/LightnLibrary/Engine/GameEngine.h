#pragma once
#include <Util/RefPtr.h>
#include <Windows.h>

class GameRenderer;
class SceneManager;
class AnimationManager;
class GraphicsResourceManager;

class GameEngine {

public:

	GameEngine();

	~GameEngine();

	void initialize(const HINSTANCE & hInst);

	void run();

	void updateGame();

private:

	std::unique_ptr<GameRenderer> _gameRenderer;
	std::unique_ptr<SceneManager> _sceneManager;
	std::unique_ptr<AnimationManager> _animationManager;
	std::unique_ptr<GraphicsResourceManager> _graphicsResourceManager;

};