#include "TitleScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include <imgui.h>
TitleScene::TitleScene() {
	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/title.png");
	logoSP_.sprite = new Sprite();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(),logoSP_.handle);
}

TitleScene::~TitleScene(){ 
	delete logoSP_.sprite;

}
void TitleScene::Initialize(){ 

	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.size = {16 * 20, 9 * 20};
	logoSP_.translate = {640, 360};
	logoSP_.sprite->SetScale(logoSP_.size);
	logoSP_.sprite->SetPosition(logoSP_.translate);
	logoSP_.sprite->Update();
	isSceneEnd_ = false;
}
void TitleScene::Update(){ 
	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
	
	isSceneEnd_ = true;
	
	}

	#ifdef USE_IMGUI
	ImGui::Begin("titleScene");
	ImGui::End();
#endif // USE_IMGUI

}
void TitleScene::Draw(){

	GameBase::GetInstance()->SpriteCommonSet();
	logoSP_.sprite->Draw();


}