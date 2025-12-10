#include "ResultScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "SceneManager.h"

ResultScene::ResultScene() {

	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/result.png");
	logoSP_.sprite = new Sprite();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), logoSP_.handle);

}

ResultScene::~ResultScene(){

}

void ResultScene::Finalize() {

	delete logoSP_.sprite; 
delete pressSpaceSprite;
}

void ResultScene::Initialize() {
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
	pressSpaceSprite->SetTextureRange({0, 0}, {768, 768});
	// 300 × 300
	pressSpaceSprite->SetScale(pressSpaceSize);

	// 画面中央より少し下（Y = 360 より下）
	pressSpaceSprite->SetPosition(pressSpacePos);

	pressSpaceSprite->Update();
}
void ResultScene::Update() {

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene("Title");
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI


}

void ResultScene::Draw(){ 
	GameBase::GetInstance()->SpriteCommonSet();
	logoSP_.sprite->Draw(); 
	pressSpaceSprite->Draw();
}