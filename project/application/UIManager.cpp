#include "UIManager.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "Sprite.h"
UIManager::UIManager(){
	
	playerHpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHP.png");
	playerHpSPData.sprite = new Sprite();
	titleSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/title.png");
	titleSPData.sprite = new Sprite();
	resultSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/result.png");
	resultSPData.sprite = new Sprite();
}
UIManager::~UIManager(){

	delete resultSPData.sprite;
	delete titleSPData.sprite;
	delete playerHpSPData.sprite;

}
void UIManager::Initialize(GameBase* gameBase){

	playerHpSPData.sprite->Initialize(gameBase->GetSpriteCommon(), playerHpSPData.handle);
	playerHpSPData.size = {100, 100};
	playerHpSPData.rotate = {0, 0};
	playerHpSPData.translate = {0, 0};
	playerHpSPData.sprite->SetScale(playerHpSPData.size);
	playerHpSPData.sprite->SetRotation(playerHpSPData.rotate);
	playerHpSPData.sprite->SetPosition(playerHpSPData.translate);
	playerHpSPData.sprite->Update();

	titleSPData.sprite->Initialize(gameBase->GetSpriteCommon(), titleSPData.handle);
	titleSPData.size = {100, 100};
	titleSPData.rotate = {0, 0};
	titleSPData.translate = {0, 0};
	titleSPData.sprite->SetScale(titleSPData.size);
	titleSPData.sprite->SetRotation(titleSPData.rotate);
	titleSPData.sprite->SetPosition(titleSPData.translate);
	titleSPData.sprite->Update();

	resultSPData.sprite->Initialize(gameBase->GetSpriteCommon(), titleSPData.handle);
	resultSPData.size = {100, 100};
	resultSPData.rotate = {0, 0};
	resultSPData.translate = {0, 0};
	resultSPData.sprite->SetScale(resultSPData.size);
	resultSPData.sprite->SetRotation(resultSPData.rotate);
	resultSPData.sprite->SetPosition(resultSPData.translate);
	resultSPData.sprite->Update();

}
void UIManager::Update() { 
	playerHpSPData.sprite->Update(); 
	titleSPData.sprite->Update();
	resultSPData.sprite->Update();
}

void UIManager::Draw(GameBase* gameBase){

	gameBase->SpriteCommonSet();
	playerHpSPData.sprite->Draw();
	titleSPData.sprite->Draw();
	resultSPData.sprite->Draw();
}