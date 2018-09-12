#include <Task/GameTask.h>

void GameTask::update(float deltaTime) {
	for (auto&& t : _childs) {
		t->update(deltaTime);
	}
}
