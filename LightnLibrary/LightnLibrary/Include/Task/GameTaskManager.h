#pragma once

#include <list>
#include <Util/Util.h>

class GameTask;

class GameTaskManager {

public:

	GameTaskManager();

	//�{���̓V�[���t�@�C����ǂݍ���ł��̃��X�g�������Ƃ��Ď󂯎��
	void initialize(const RefPtr<GameTask>& rootTask);

	void updateTasks(float deltaTime);


private:
	RefPtr<GameTask> _sceneTask;

};