#include "Game.h"

void Game::Initialize(){
	FrameWork::Initialize();
	GameBase::GetInstance()->Initialize(L"LE2A_04_カシワギハルカ", 1280, 720);
	d3dResourceLeakChecker = new D3DResourceLeakChecker();
	// エンジンの初期化
	titleScene = new TitleScene();
	titleScene->Initialize();
	gameScene = new GameScene();
	gameScene->Initialize();
	resultScene = new ResultScene();
	resultScene->Initialize();
	SetUnhandledExceptionFilter(GameBase::GetInstance()->ExportDump);
	scene = SceneName::Title;
#ifdef _DEBUG
	scene = SceneName::Game;
#endif // DEBUG
}

void Game::Update(){
	FrameWork::Update();
	GameBase::GetInstance()->BeginFlame();

	// update
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
		if (gameScene->IsSceneEnd_GameClear() || gameScene->IsSceneEnd_GameOver()) {
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

	if (!GameBase::GetInstance()->ProcessMessage()) {
		endRequest_ = true;
	}
}

void Game::Draw() {

	// draw
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
	// ゲームの処理
	GameBase::GetInstance()->EndFlame();

}

void Game::Finalize() {
	delete resultScene;
	delete gameScene;
	delete titleScene;
	GameBase::GetInstance()->Finalize();
	delete d3dResourceLeakChecker;
	CoUninitialize();
	FrameWork::Finalize();
}