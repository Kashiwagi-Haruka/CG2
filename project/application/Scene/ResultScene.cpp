#include "ResultScene.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "SceneManager.h"
#include "TitleScene.h"
ResultScene::ResultScene() {

	logoSP_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/result.png");
	logoSP_.sprite = new Sprite();
	logoSP_.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), logoSP_.handle);

}

ResultScene::~ResultScene(){

}

void ResultScene::Finalize() {

	delete logoSP_.sprite; }

void ResultScene::Initialize() {
	logoSP_.sprite->SetAnchorPoint({0.5f, 0.5f});
	logoSP_.size = {16 * 20, 9 * 20};
	logoSP_.translate = {640, 360};
	logoSP_.sprite->SetScale(logoSP_.size);
	logoSP_.sprite->SetPosition(logoSP_.translate);
	logoSP_.sprite->Update();
	isSceneEnd_ = false;

}
void ResultScene::Update() {

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {
		BaseScene* scene = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(scene);
		
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI


}

void ResultScene::Draw(){ 
	GameBase::GetInstance()->SpriteCommonSet();
	logoSP_.sprite->Draw(); 
}