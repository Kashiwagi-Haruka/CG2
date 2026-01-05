#include "GameOverScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "SceneManager.h"

GameOverScene::GameOverScene() {

	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/over.png");
	logoSP_.sprite = std::make_unique<Sprite>();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), logoSP_.handle);
}

void GameOverScene::Finalize() {
	
}

void GameOverScene::Initialize() {

	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.size = {1280, 720};
	logoSP_.translate = {640, 360};
	logoSP_.sprite->SetScale(logoSP_.size);
	logoSP_.sprite->SetPosition(logoSP_.translate);
	logoSP_.sprite->Update();

	pressSpaceHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");

	pressSpaceSprite = std::make_unique<Sprite>();
	pressSpaceSprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), pressSpaceHandle);
	pressSpaceSprite->SetAnchorPoint({0.5f, 0.5f});
	pressSpaceSprite->SetScale(pressSpaceSize);
	pressSpaceSprite->SetPosition(pressSpacePos);
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	pressSpaceSprite->Update();
	GameBase::GetInstance()->SetIsCursorStablity(false);
	GameBase::GetInstance()->SetIsCursorVisible(true);
}

void GameOverScene::Update() {

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene("Title");
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI
}

void GameOverScene::Draw() {
	GameBase::GetInstance()->SpriteCommonSet();
	logoSP_.sprite->Draw();
	pressSpaceSprite->Draw();
}