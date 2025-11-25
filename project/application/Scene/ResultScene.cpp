#include "ResultScene.h"
#include "GameBase.h"
void ResultScene::Initialize() {

	isSceneEnd_ = false;

}
void ResultScene::Update(bool isGameClear, bool isGameOver) {

	if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {

		isSceneEnd_ = true;
		
	}
#ifdef USE_IMGUI
	ImGui::Begin("resultScene");
	ImGui::End();
#endif // USE_IMGUI


}

void ResultScene::Draw(){

}