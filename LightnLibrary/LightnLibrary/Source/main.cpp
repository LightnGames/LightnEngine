#include <Windows.h>

#include <Util/Input.h>
#include <GameEngine.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, int iCmdShow) {

	GameEngine* engine = new GameEngine();
	engine->initialize(hInst);
	engine->run();

	return 0;
}