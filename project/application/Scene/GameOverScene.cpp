#include "GameOverScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
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
	sprite_->SetScale({ static_cast<float>(WinApp::kClientWidth),static_cast<float>(WinApp::kClientHeight)  });
	sprite_->Update();
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

	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}