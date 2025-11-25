#include "SceneTransition.h"
#include "TextureManager.h"
#include "GameBase.h"
SceneTransition::SceneTransition(){
	
	fadeSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png");
	fadeSPData.sprite = new Sprite();
	fadeSPData.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), fadeSPData.handle);
}
SceneTransition::~SceneTransition(){ 
	delete fadeSPData.sprite; 
}

void SceneTransition::Initialize(){ 
	
	fadeSPData.size = {1280, 720};
	fadeSPData.translate = {0, 0}; 
	fadeSPData.sprite->SetScale(fadeSPData.size);
	fadeSPData.sprite->SetPosition(fadeSPData.translate);
	fadeSPData.sprite->Update();
	isEnd = false;
}
void SceneTransition::Update(bool isIn) { 

	fadeSPData.sprite->SetColor({0, 0, 0, color});
	if (isIn) {
		color -= 0.01f;
		if (color <= 0.0f) {
			color = 0.0f;
			isEnd = true;
		}
	} else {
		color += 0.01f;
		if (color >= 1.0f) {
			color = 1.0f;
			isEnd = true;
		}
	}

}
void SceneTransition::Draw() { fadeSPData.sprite->Draw(); }