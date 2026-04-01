#include "GameOverScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "ScreenSize/ScreenSize.h"
#include "Sprite/SpriteCommon.h"
#include "Text/FreetypeManager/FreeTypeManager.h"
#include "TextureManager.h"

GameOverScene::GameOverScene() {
	transition = std::make_unique<SceneTransition>();
	BGM_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/BGM/gameOverBGM.mp3");

	textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/gameOverImage.png");

	sprite_ = std::make_unique<Sprite>();
}

void GameOverScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGM_); }

void GameOverScene::Initialize() {

	sprite_->Initialize(textureHandle_);
	sprite_->SetScale({SCREEN_SIZE::WIDTH, SCREEN_SIZE::HEIGHT});
	sprite_->Update();

	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Bold.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 96, 96);
	gameOverText_.Initialize(fontHandle_);
	gameOverText_.SetString(U"GameOver");
	gameOverText_.SetPosition({SCREEN_SIZE::WIDTH * 0.5f, SCREEN_SIZE::HEIGHT * 0.25f});
	gameOverText_.SetColor({1.0f, 0.2f, 0.2f, 1.0f});
	gameOverText_.SetAlign(TextAlign::Center);
	gameOverText_.UpdateLayout(false);

	transition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	isBGMPlaying = false;
}
void GameOverScene::Update() {

	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGM_, true);
		isBGMPlaying = true;
	}
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isTransitionOut) {
		transition->Initialize(true);
		isTransitionOut = true;
	}
	if (isTransitionIn || isTransitionOut) {
		transition->Update();
		if (transition->IsEnd() && isTransitionIn) {
			isTransitionIn = false;
		}
		if (transition->IsEnd() && isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene("Title");
		}
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI
}

void GameOverScene::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	sprite_->Draw();
	SpriteCommon::GetInstance()->DrawCommonFont();
	gameOverText_.Draw();
	FreeTypeManager::ResetFontUsage();

	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}