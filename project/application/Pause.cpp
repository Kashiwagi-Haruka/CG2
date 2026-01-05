#include "Pause.h"
#include "TextureManager.h"
#include "GameBase.h"
#include "Function.h"
Pause::Pause(){
	BGHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/PauseBG.png");
	SelectHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/PauseSelect.png");
	ButtonHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/PauseButton.png");

	BG_ = std::make_unique<Sprite>();
	Select_ = std::make_unique<Sprite>();
	Button_ = std::make_unique<Sprite>();
	
	BG_->Initialize(GameBase::GetInstance()->GetSpriteCommon(), BGHandle_);
	Select_->Initialize(GameBase::GetInstance()->GetSpriteCommon(), SelectHandle_);
	Button_->Initialize(GameBase::GetInstance()->GetSpriteCommon(), ButtonHandle_);
}

void Pause::Initialize(){

	startTime = 0;
	isStart = true;
	isSelect = true;
}

void Pause::Update(bool isPause) { 

	float pos;

	if (isStart) {
		startTime += 0.01f;
		pos = Function::Lerp(1280, 0, startTime);

		if (startTime>1.0f) {
			isStart = false;

		}
	} else {
		if (isSelect) {
		
		} else {
			pos = Function::Lerp(0, 1280, startTime);	
		}
	}

	BG_->Update();
	Select_->Update();
	Button_->Update();
}

void Pause::Draw(){

	BG_->Draw();
	Select_->Draw();
	Button_->Draw();

}