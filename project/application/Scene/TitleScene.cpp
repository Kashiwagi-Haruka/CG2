#include "TitleScene.h"
#include "GameBase.h"
#include <imgui.h>
TitleScene::~TitleScene(){


}
void TitleScene::Initialize(){ 
	isSceneEnd_ = false;
}
void TitleScene::Update(GameBase* gameBase){ 
	if (gameBase->TriggerKey(DIK_SPACE)) {
	
	isSceneEnd_ = true;
	
	}

	#ifdef USE_IMGUI
	ImGui::Begin("titleScene");
	ImGui::End();
#endif // USE_IMGUI

}
void TitleScene::Draw(){

}