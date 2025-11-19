#include"GameBase.h"
#include "D3DResourceLeakChecker.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TitleScene.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	GameBase* gameBase=new GameBase;
	D3DResourceLeakChecker* d3dResourceLeakChecker = new D3DResourceLeakChecker();
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	TitleScene* titleScene = new TitleScene();
	titleScene->Initialize();
	GameScene* gameScene = new GameScene();
	gameScene->Initialize(gameBase);
	ResultScene* resultScene = new ResultScene();
	resultScene->Initialize(gameBase);
	SetUnhandledExceptionFilter(gameBase->ExportDump);
	
	enum class SceneName {

		Title,
		Game,
		Result

	};
	SceneName scene=SceneName::Title;
	/*#ifdef _DEBUG*/
		scene = SceneName::Game;
	//#endif // DEBUG
	/*int PrePressMouse = 0;*/
	while (gameBase->ProcessMessage()) {
		gameBase->BeginFlame();

		//update
		switch (scene) {
		case SceneName::Title:
			titleScene->Update(gameBase);
			if (titleScene->GetIsSceneEnd()) {
				scene = SceneName::Game;
				gameScene->Initialize(gameBase);
			}

			break;
		case SceneName::Game:
			gameScene->Update(gameBase);
			if (gameScene->IsSceneEnd_GameClear()||gameScene->IsSceneEnd_GameOver()) {
				scene = SceneName::Result;
				resultScene->Initialize(gameBase);
			}
			break;
		case SceneName::Result:
			resultScene->Update(gameScene->IsSceneEnd_GameClear(), gameScene->IsSceneEnd_GameOver(), gameBase);
			if (resultScene->GetIsSceneEnd()) {
				scene = SceneName::Title;
				titleScene->Initialize();
			}
			break;
		default:
			break;
		}
		//draw
		switch (scene) {
		case SceneName::Title:
			titleScene->Draw();
			break;
		case SceneName::Game:
			gameScene->Draw(gameBase);
			break;
		case SceneName::Result:
			resultScene->Draw();
			break;
		default:
			break;
		}
		//ゲームの処理
		gameBase->EndFlame();
		
	}

	delete resultScene;
	delete gameScene;
	delete titleScene;
	delete gameBase;
	delete d3dResourceLeakChecker;
	CoUninitialize();

	return 0;

}

