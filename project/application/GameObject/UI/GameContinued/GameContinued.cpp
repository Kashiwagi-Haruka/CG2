#include "GameContinued.h"
#include "GameObject/KeyBindConfig.h"
#include "TextureManager.h"

void GameContinued::Initialize() {
	for (auto& saveData : gameSaveData) {
		saveData.GameSceneSprite_ = std::make_unique<Sprite>();
		saveData.BlockSprite_ = std::make_unique<Sprite>();
	}
	text_ = std::make_unique<GameContinuedText>();
	text_->Initialize();

	for (int i = 0; i < saveDataMaxNum_; ++i) {
		SetSaveData(i, "Save " + std::to_string(i + 1), "No Data", "--:--");
	}

	currentSelectNum_ = 0;
	isSelected_ = false;
}

void GameContinued::Update() {
	PlayerCommand* command = PlayerCommand::GetInstance();
	if (command->UiMoveForwardTrigger() || command->MouseWheelDown()) {
		if (currentSelectNum_ > 0) {
			--currentSelectNum_;
		}
	}

	if (command->UiMoveBackwardTrigger() || command->MouseWheelUp()) {
		if (currentSelectNum_ < saveDataMaxNum_ - 1) {
			++currentSelectNum_;
		}
	}

	if (command->UiInteractTrigger()) {
		isSelected_ = true;
	}

	text_->Update(currentSelectNum_);
}

void GameContinued::Draw() { text_->Draw(); }

void GameContinued::SetSaveData(int index, const std::string& name, const std::string& currentStageName, const std::string& saveDateTime) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}

	gameSaveData[index].Name_ = name;
	gameSaveData[index].currentStageName_ = currentStageName;
	gameSaveData[index].saveDateTime_ = saveDateTime;

	if (text_) {
		text_->SetSaveDataText(name, currentStageName, saveDateTime, index);
	}
}