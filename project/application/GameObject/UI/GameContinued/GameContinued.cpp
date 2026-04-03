#include "GameContinued.h"
#include "GameObject/KeyBindConfig.h"
#include "TextureManager.h"
#include "SpriteCommon.h"

namespace {
constexpr float kBlockStartX = 120.0f;
constexpr float kBlockStartY = 120.0f;
constexpr float kBlockWidth = 1040.0f;
constexpr float kBlockHeight = 88.0f;
constexpr float kBlockSpacingY = 110.0f;
} // namespace

void GameContinued::Initialize() {
	int blockTextureHandle_ = 0;
	int screenTextureHandle_ = 0;
	blockTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");

	for (auto& saveData : gameSaveData) {
		screenTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/SaveScreenShot/NoData.png");
		saveData.GameSceneSprite_ = std::make_unique<Sprite>();
		saveData.GameSceneSprite_->Initialize(screenTextureHandle_);
		saveData.BlockSprite_ = std::make_unique<Sprite>();
		saveData.BlockSprite_->Initialize(blockTextureHandle_);
		saveData.BlockSprite_->SetColor({0.08f, 0.08f, 0.08f, 0.8f});
	}
	text_ = std::make_unique<GameContinuedText>();
	text_->Initialize();

	for (int i = 0; i < saveDataMaxNum_; ++i) {
		SetSaveData(i, "Save " + std::to_string(i + 1), "No Data", "--:--");

		const float y = kBlockStartY + (kBlockSpacingY * static_cast<float>(i));
		gameSaveData[i].BlockSprite_->SetPosition({kBlockStartX, y});
		gameSaveData[i].BlockSprite_->SetScale({kBlockWidth, kBlockHeight});
		gameSaveData[i].BlockSprite_->Update();
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

	for (int i = 0; i < saveDataMaxNum_; ++i) {
		const bool isCurrent = (i == currentSelectNum_);
		gameSaveData[i].BlockSprite_->SetColor(isCurrent ? Vector4{0.35f, 0.12f, 0.12f, 0.92f} : Vector4{0.08f, 0.08f, 0.08f, 0.8f});
		gameSaveData[i].BlockSprite_->Update();
	}

	text_->Update(currentSelectNum_);
}

void GameContinued::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	for (auto& saveData : gameSaveData) {
		saveData.BlockSprite_->Draw();
	}
	text_->Draw();
}

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