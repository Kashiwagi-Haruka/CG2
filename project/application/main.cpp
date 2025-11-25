#include"GameBase.h"
#include "D3DResourceLeakChecker.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TitleScene.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	GameBase::GetInstance()->Initialize(L"CG2", 1280, 720);
	D3DResourceLeakChecker* d3dResourceLeakChecker = new D3DResourceLeakChecker();
	//エンジンの初期化
	TitleScene* titleScene = new TitleScene();
	titleScene->Initialize();
	GameScene* gameScene = new GameScene();
	gameScene->Initialize();
	ResultScene* resultScene = new ResultScene();
	resultScene->Initialize();
	SetUnhandledExceptionFilter(GameBase::GetInstance()->ExportDump);
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
	while (GameBase::GetInstance()->ProcessMessage()) {
		GameBase::GetInstance()->BeginFlame();

		//update
		switch (scene) {
		case SceneName::Title:
			titleScene->Update();
			if (titleScene->GetIsSceneEnd()) {
				scene = SceneName::Game;
				gameScene->Initialize();
			}

			break;
		case SceneName::Game:
			gameScene->Update();
			if (gameScene->IsSceneEnd_GameClear()||gameScene->IsSceneEnd_GameOver()) {
				scene = SceneName::Result;
				resultScene->Initialize();
			}
			break;
		case SceneName::Result:
			resultScene->Update(gameScene->IsSceneEnd_GameClear(), gameScene->IsSceneEnd_GameOver());
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
			gameScene->Draw();
			break;
		case SceneName::Result:
			resultScene->Draw();
			break;
		default:
			break;
		}
		//ゲームの処理
		GameBase::GetInstance()->EndFlame();
		
	}

	delete resultScene;
	delete gameScene;
	delete titleScene;
	GameBase::GetInstance()->Finalize();
	delete d3dResourceLeakChecker;
	CoUninitialize();

	return 0;

}

