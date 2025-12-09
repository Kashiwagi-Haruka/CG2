#include "GameOverScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "SceneManager.h"
#include "TitleScene.h"
GameOverScene::GameOverScene() {

	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/over.png");
	logoSP_.sprite = new Sprite();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), logoSP_.handle);
}

GameOverScene::~GameOverScene() {}

void GameOverScene::Finalize() { delete logoSP_.sprite;
	delete pressSpaceSprite;
}

void GameOverScene::Initialize() {
	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.size = {1280, 720};
	logoSP_.translate = {640, 360};
	logoSP_.sprite->SetScale(logoSP_.size);
	logoSP_.sprite->SetPosition(logoSP_.translate);
	logoSP_.sprite->Update();
	isSceneEnd_ = false;
	// SPACE 画像読み込み
	pressSpaceHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");

	pressSpaceSprite = new Sprite();
	pressSpaceSprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), pressSpaceHandle);

	// 中央寄せ
	pressSpaceSprite->SetAnchorPoint({0.5f, 0.5f});

	// 300 × 300
	pressSpaceSprite->SetScale(pressSpaceSize);

	// 画面中央より少し下（Y = 360 より下）
	pressSpaceSprite->SetPosition(pressSpacePos);
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	pressSpaceSprite->Update();
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
	pressSpaceSprite->Draw();
}