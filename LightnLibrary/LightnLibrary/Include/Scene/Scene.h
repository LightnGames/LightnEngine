#pragma once

#include <Task/GameTask.h>

class Scene :public GameTask{

public:

	Scene();

	virtual void update(float deltaTime);

	~Scene();

};