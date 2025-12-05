#include "Scene/Game/Game.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	FrameWork* game = new Game();
	
	game->Run();

	delete game;

	return 0;

}