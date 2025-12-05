#include "Game.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TitleScene.h"
#include "SceneManager.h"
void Game::Initialize(){
	FrameWork::Initialize();
	GameBase::GetInstance()->Initialize(L"LE2A_04_カシワギハルカ", 1280, 720);
	d3dResourceLeakChecker = new D3DResourceLeakChecker();

	

	SetUnhandledExceptionFilter(GameBase::GetInstance()->ExportDump);
	
#ifdef _DEBUG
	
	BaseScene *scene_ = new GameScene();
#endif // DEBUG
	SceneManager::GetInstance()->SetNextScene(scene_);
}

void Game::Update(){
	FrameWork::Update();
	GameBase::GetInstance()->BeginFlame();



	SceneManager::GetInstance()->Update();
	if (!GameBase::GetInstance()->ProcessMessage()) {
		endRequest_ = true;
	}
}

void Game::Draw() {
	SceneManager::GetInstance()->Draw();
	
	// ゲームの処理
	GameBase::GetInstance()->EndFlame();

}

void Game::Finalize() {
	
	SceneManager::GetInstance()->Finalize();
	GameBase::GetInstance()->Finalize();
	delete d3dResourceLeakChecker;
	CoUninitialize();
	FrameWork::Finalize();
}