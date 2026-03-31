#include "ResultScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include"ScreenSize/ScreenSize.h"

ResultScene::ResultScene() {

	BGM_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/BGM/kimigayo01.mp3");

	transition = std::make_unique<SceneTransition>();

	textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/kimigayo.png");

	sprite_ = std::make_unique<Sprite>();
}

void ResultScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGM_); }

void ResultScene::Initialize() {
	isSceneEnd_ = false;
	sprite_->Initialize(textureHandle_);
	sprite_->SetScale({SCREEN_SIZE::WIDTH,SCREEN_SIZE::HEIGHT});
	sprite_->Update();
	Input::GetInstance()->SetIsCursorStability(false);
	Input::GetInstance()->SetIsCursorVisible(true);
	transition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	isBGMPlaying = false;
}
void ResultScene::Update() {
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

void ResultScene::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	sprite_->Draw();
	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}