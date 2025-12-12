#include "TitleScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include <imgui.h>
#include "SceneManager.h"
TitleScene::TitleScene() {
	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/title.png");
	logoSP_.sprite = std::make_unique<Sprite>();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), logoSP_.handle);
}

void TitleScene::Finalize() {
	
}

void TitleScene::Initialize() {

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
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	pressSpaceSprite->SetScale(pressSpaceSize);
	pressSpaceSprite->SetPosition(pressSpacePos);
	pressSpaceSprite->Update();
}

void TitleScene::Update(){ 
	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
	
	SceneManager::GetInstance()->ChangeScene("Game");
	}

	#ifdef USE_IMGUI
	ImGui::Begin("titleScene");
	ImGui::End();
#endif // USE_IMGUI

}
void TitleScene::Draw(){

	GameBase::GetInstance()->SpriteCommonSet();
	logoSP_.sprite->Draw();
	pressSpaceSprite->Draw();


}