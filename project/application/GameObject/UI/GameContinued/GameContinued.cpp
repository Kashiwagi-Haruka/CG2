#include "GameContinued.h"
#include "TextureManager.h"
void GameContinued::Initialize() {
	for (auto& saveData : gameSaveData) {
		saveData.GameSceneSprite_ = std::make_unique<Sprite>();
		saveData.BlockSprite_ = std::make_unique<Sprite>();
	}
	text_ = std::make_unique<GameContinuedText>();
	text_->Initialize();
}
void GameContinued::Update() { text_->Update(); }
void GameContinued::Draw() { text_->Draw(); }