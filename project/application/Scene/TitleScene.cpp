#include "TitleScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include <imgui.h>
#include "SceneManager.h"
TitleScene::TitleScene() {
	BGSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/title.png");
	BGSP_.sprite = std::make_unique<Sprite>();
	BGSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), BGSP_.handle);
	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/logo.png");
	logoSP_.sprite = std::make_unique<Sprite>();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(),logoSP_.handle);
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Rendez-vous_2.mp3");
	transition = std::make_unique<SceneTransition>();
}

void TitleScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGMData); }

void TitleScene::Initialize() {

	BGSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	BGSP_.size = {2000, 2000};
	BGSP_.translate = {640, 0};
	BGSP_.sprite->SetScale(BGSP_.size);
	BGSP_.sprite->SetPosition(BGSP_.translate);
	BGSP_.sprite->Update();
	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.sprite->SetPosition({640, 300});
	logoSP_.sprite->SetScale({500, 500});
	logoSP_.sprite->Update();

	pressSpaceHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");

	pressSpaceSprite = std::make_unique<Sprite>();
	pressSpaceSprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), pressSpaceHandle);
	pressSpaceSprite->SetAnchorPoint({0.5f, 0.5f});
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	pressSpaceSprite->SetScale(pressSpaceSize);
	pressSpaceSprite->SetPosition(pressSpacePos);
	pressSpaceSprite->Update();
	isBGMPlaying = false;
	isTransition = false;
	GameBase::GetInstance()->SetIsCursorStablity(false);
	GameBase::GetInstance()->SetIsCursorVisible(true);
}

void TitleScene::Update(){ 
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}

	BGSP_.rotate+=0.01f;
	
	BGSP_.sprite->SetRotation(BGSP_.rotate);
	BGSP_.sprite->Update();
	

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
		transition->Initialize(true);
		isTransition = true;
	}
	if (isTransition) {
		transition->Update();
		if (transition->IsEnd()) {

			SceneManager::GetInstance()->ChangeScene("Game");
		}
	}

	#ifdef USE_IMGUI
	ImGui::Begin("titleScene");
	ImGui::End();
#endif // USE_IMGUI

}
void TitleScene::Draw(){

	GameBase::GetInstance()->SpriteCommonSet();
	BGSP_.sprite->Draw();
	logoSP_.sprite->Draw();
	pressSpaceSprite->Draw();
	if (isTransition) {
		transition->Draw();
	}

}