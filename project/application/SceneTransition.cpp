#include "SceneTransition.h"
#include "TextureManager.h"
#include "GameBase.h"
SceneTransition::SceneTransition(){
	
	fadeSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/transition.png");
	fadeSPData.sprite = std::make_unique<Sprite>();
	fadeSPData.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), fadeSPData.handle);
}
SceneTransition::~SceneTransition(){ 
	
}

void SceneTransition::Initialize(bool isIn){ 
	
	isIn_ = isIn;
	if (isIn_) {
		fadeSPData.size = {0, 720};
		fadeSPData.translate = {-1500, 0}; 
	} else {
		fadeSPData.size = {1500, 720};
		fadeSPData.translate = {0, 0}; 
	}
	
	fadeSPData.sprite->SetScale(fadeSPData.size);
	fadeSPData.sprite->SetPosition(fadeSPData.translate);
	fadeSPData.sprite->Update();
	
	isEnd = false;
}
void SceneTransition::Update() { 

	
	if (isIn_) {
		fadeSPData.size.x += 10.0f;
		fadeSPData.translate.x -= 10.0f;
		if (fadeSPData.size.x >= 1500.0f) {
			fadeSPData.size.x = 1500.0f;
			isEnd = true;
		}
	} else {
		fadeSPData.size.x -= 10.0f;
		fadeSPData.translate.x += 10.0f;
		if (fadeSPData.size.x <=  0.0f) {
			fadeSPData.size.x = 0.0f;
			isEnd = true;
		}
	}
	fadeSPData.sprite->SetScale(fadeSPData.size);
	fadeSPData.sprite->SetPosition(fadeSPData.translate);
	fadeSPData.sprite->Update();
}
void SceneTransition::Draw() { fadeSPData.sprite->Draw(); }