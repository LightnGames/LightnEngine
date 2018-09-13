#pragma once

#include <Task/GameTask.h>

class Scene :public GameTask{

public:

	Scene();
	~Scene();

	virtual void start() override {};

	virtual void update(float deltaTime) override;

protected:

	void loadSceneAsset(const std::string& fileName);

};