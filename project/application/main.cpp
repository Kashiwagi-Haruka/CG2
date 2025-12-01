#include "Scene/Game.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	Game game;
	game.Initialize();
	while (GameBase::GetInstance()->ProcessMessage()) {
		
		game.Update();

		if (game.isEndRequest()) {
			break;
		}
		game.Draw();
		
	}

	game.Finalize();

	return 0;

}