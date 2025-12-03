#include "UIManager.h"
#include "GameBase.h"
#include "TextureManager.h"
#include "Sprite.h"
UIManager::UIManager(){
	
	playerHpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHP.png");
	playerHPFlameSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHPFlame.png");
	HowtoOperateSPData[kW].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/WASD.png");
	HowtoOperateSPData[kA].handle = HowtoOperateSPData[kW].handle;
	HowtoOperateSPData[kS].handle = HowtoOperateSPData[kW].handle;
	HowtoOperateSPData[kD].handle = HowtoOperateSPData[kW].handle;
	HowtoOperateSPData[kSpace].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");
	

	playerHpSPData.sprite = new Sprite();
	playerHPFlameSPData.sprite = new Sprite();
	for (int i = 0; i < CountMAX; i++) {
	HowtoOperateSPData[i].sprite = new Sprite();
	}
}
UIManager::~UIManager(){
	for (int i = 0; i < CountMAX; i++) {
		delete HowtoOperateSPData[i].sprite;
	}
	delete playerHPFlameSPData.sprite;
	delete playerHpSPData.sprite;

}
void UIManager::Initialize(){

	playerHpSPData.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), playerHpSPData.handle);
	playerHpSPData.size = playerHPMaxSize;
	playerHpSPData.translate = {0, 0};
	playerHpSPData.sprite->SetScale(playerHpSPData.size);
	playerHpSPData.sprite->SetPosition(playerHpSPData.translate);
	playerHpSPData.sprite->Update();

	playerHPFlameSPData.sprite->Initialize(GameBase::GetInstance()->GetSpriteCommon(), playerHPFlameSPData.handle);
	playerHPFlameSPData.size = {40, 70};
	playerHPFlameSPData.translate = {playerHpSPData.translate.x - 10, playerHpSPData.translate.y - 10};
	playerHPFlameSPData.sprite->SetScale(playerHPFlameSPData.size);
	playerHPFlameSPData.sprite->SetPosition(playerHPFlameSPData.translate);
	playerHPFlameSPData.sprite->Update();
	
}
void UIManager::Update() {
	playerHpSPData.size.y = (playerHPMaxSize.y / playerHPMax) * playerHP;
	playerHpSPData.sprite->SetScale(playerHpSPData.size);
	playerHpSPData.sprite->SetPosition(playerHpSPData.translate);
	playerHpSPData.sprite->Update(); 
	playerHPFlameSPData.translate = {playerHpSPData.translate.x - 10, playerHpSPData.translate.y - 10};
	playerHPFlameSPData.sprite->SetPosition(playerHPFlameSPData.translate);
	playerHPFlameSPData.sprite->Update();
}

void UIManager::Draw(){

	GameBase::GetInstance()->SpriteCommonSet();
	playerHpSPData.sprite->Draw();
	playerHPFlameSPData.sprite->Draw();
}
void UIManager::SetPlayerPosition(Vector2 playerPosition){ 
	playerHpSPData.translate.x = playerPosition.x+600;
	playerHpSPData.translate.y = playerPosition.y + 500;
}
void UIManager::SetPlayerHP(int HP) { playerHP = HP; }
void UIManager::SetPlayerHPMax(int HPMax) { playerHPMax = HPMax; }