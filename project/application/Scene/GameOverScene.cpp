#include "GameOverScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "SceneManager.h"
#include "TitleScene.h"
GameOverScene::GameOverScene() {

	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/result.png");
	logoSP_.sprite = new Sprite();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), logoSP_.handle);
}

GameOverScene::~GameOverScene() {}

void GameOverScene::Finalize() { delete logoSP_.sprite; }

void GameOverScene::Initialize() {
	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.size = {16 * 20, 9 * 20};
	logoSP_.translate = {640, 360};
	logoSP_.sprite->SetScale(logoSP_.size);
	logoSP_.sprite->SetPosition(logoSP_.translate);
	logoSP_.sprite->Update();
	isSceneEnd_ = false;
}
void GameOverScene::Update() {

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
		BaseScene* scene = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(scene);
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI
}

void GameOverScene::Draw() {
	GameBase::GetInstance()->SpriteCommonSet();
	logoSP_.sprite->Draw();
}