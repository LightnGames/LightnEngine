#pragma once

#include <list>
#include <Util/RefPtr.h>

class GameTask;

class GameTaskManager {

public:

	GameTaskManager();

	//本当はシーンファイルを読み込んでそのリストを引数として受け取る
	void initialize(const RefPtr<GameTask>& rootTask);

	void updateTasks(float deltaTime);


private:
	RefPtr<GameTask> _sceneTask;

};