#include "GameContinued.h"
#include "DirectXCommon.h"
#include "GameObject/KeyBindConfig.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kBlockStartX = 120.0f;
constexpr float kBlockStartY = 120.0f;
constexpr float kBlockWidth = 1040.0f;
constexpr float kBlockHeight = 140.0f;
constexpr float kBlockSpacingY = 156.0f;
constexpr float kSelectedCenterOffset = (saveDataMaxNum_ - 1) * 0.5f;
constexpr float kBlockCenterX = kBlockStartX + (kBlockWidth * 0.5f);
constexpr float kBlockCenterY = kBlockStartY + (kBlockHeight * 0.5f);

float Lerp(float start, float end, float t) { return start + (end - start) * t; }

Vector2 Lerp(const Vector2& start, const Vector2& end, float t) { return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t)}; }

Vector4 Lerp(const Vector4& start, const Vector4& end, float t) { return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t), Lerp(start.z, end.z, t), Lerp(start.w, end.w, t)}; }
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
		saveData.BlockSprite_->SetAnchorPoint({0.5f, 0.5f});
	}
	text_ = std::make_unique<GameContinuedText>();
	text_->Initialize();

	for (int i = 0; i < saveDataMaxNum_; ++i) {
		SetSaveData(i, "Save " + std::to_string(i + 1), "No Data", "--:--");

		const float y = kBlockCenterY + (kBlockSpacingY * static_cast<float>(i));
		blockPositions_[i] = {kBlockCenterX, y};
		gameSaveData[i].BlockSprite_->SetPosition(blockPositions_[i]);
		gameSaveData[i].BlockSprite_->SetScale({kBlockWidth, kBlockHeight});
		blockColors_[i] = {0.3f, 0.3f, 0.3f, 0.86f};
		blockScales_[i] = {kBlockWidth, kBlockHeight};
		gameSaveData[i].BlockSprite_->SetColor(blockColors_[i]);
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
		const float maxDistance = static_cast<float>(saveDataMaxNum_ - 1);
		const float distance = std::abs(static_cast<float>(i - currentSelectNum_));
		const float distanceRate = distance / maxDistance;
		const float easedDistance = distanceRate * distanceRate;
		const float proximity = 1.0f - distanceRate;

		const float targetY = kBlockCenterY + (kBlockSpacingY * (static_cast<float>(i) - static_cast<float>(currentSelectNum_) + kSelectedCenterOffset));
		const Vector2 targetPosition = {kBlockCenterX, targetY};

		const float r = 0.15f + (proximity * 0.80f);
		const float g = 0.18f + (proximity * 0.57f);
		const float b = 0.22f + (proximity * 0.25f);
		const float alpha = 0.78f + (proximity * 0.22f);
		const Vector4 targetColor = {r, g, b, alpha};

		const float scaleFactor = 0.72f + (std::pow(1.0f - easedDistance, 1.4f) * 0.42f);
		const Vector2 targetScale = {kBlockWidth * scaleFactor, kBlockHeight * scaleFactor};

		const float deltaTime = SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
		const float transitionRate = std::clamp(deltaTime * 8.0f, 0.0f, 1.0f);
		blockPositions_[i] = Lerp(blockPositions_[i], targetPosition, transitionRate);
		blockColors_[i] = Lerp(blockColors_[i], targetColor, transitionRate);
		blockScales_[i] = Lerp(blockScales_[i], targetScale, transitionRate);

		gameSaveData[i].BlockSprite_->SetPosition(blockPositions_[i]);
		gameSaveData[i].BlockSprite_->SetColor(blockColors_[i]);
		gameSaveData[i].BlockSprite_->SetScale(blockScales_[i]);
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