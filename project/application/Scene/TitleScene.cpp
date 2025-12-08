#include "TitleScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include <imgui.h>
#include "SceneManager.h"
#include "GameScene.h"
TitleScene::TitleScene() {
	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/title.png");
	logoSP_.sprite = new Sprite();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(),logoSP_.handle);
}

TitleScene::~TitleScene(){ 
	

}
void TitleScene::Finalize(){ 
	delete logoSP_.sprite; 
	delete pressSpaceSprite;
}
void TitleScene::Initialize(){ 

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
	pressSpaceSprite->SetTextureRange({0,0},{768, 768});
	// 300 × 300
	pressSpaceSprite->SetScale(pressSpaceSize);

	// 画面中央より少し下（Y = 360 より下）
	pressSpaceSprite->SetPosition(pressSpacePos);

	pressSpaceSprite->Update();
}
void TitleScene::Update(){ 
	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
	
	BaseScene* scene = new GameScene();
		SceneManager::GetInstance()->SetNextScene(scene);
	
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