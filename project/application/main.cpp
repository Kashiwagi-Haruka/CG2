#include"GameBase.h"
#include "D3DResourceLeakChecker.h"
#include "GameScene.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	GameBase* gameBase=new GameBase;
	D3DResourceLeakChecker* d3dResourceLeakChecker = new D3DResourceLeakChecker();
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	GameScene* gameScene = new GameScene();
	gameScene->Initialize(gameBase);
	SetUnhandledExceptionFilter(gameBase->ExportDump);
	SoundData soundData1 = gameBase->SoundLoadWave("Resources/audio/Alarm01.wav");
	gameBase->SoundPlayWave(soundData1);
	enum class SceneName {

		Title,
		Game,

	};
	SceneName scene=SceneName::Title;
	#ifdef _DEBUG
		scene = SceneName::Game;
	#endif // DEBUG
	/*int PrePressMouse = 0;*/
	while (gameBase->ProcessMessage()) {
		gameBase->BeginFlame();

		//update
		switch (scene) {
		case SceneName::Title:
			break;
		case SceneName::Game:
			gameScene->Update(gameBase);
			break;
		default:
			break;
		}
		//draw
		switch (scene) {
		case SceneName::Title:
			break;
		case SceneName::Game:
			gameScene->Draw(gameBase);
			break;
		default:
			break;
		}
		//ゲームの処理
		gameBase->EndFlame();
		
	}

	
	gameBase->SoundUnload(&soundData1);
	delete gameScene;
	delete gameBase;
	delete d3dResourceLeakChecker;
	CoUninitialize();

	return 0;

}

