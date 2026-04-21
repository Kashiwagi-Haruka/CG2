#define NOMINMAX
#include "GameContinued.h"
#include "DirectXCommon.h"
#include "GameObject/KeyBindConfig.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "application/Color/Color.h"
#include <algorithm>
#include"GameObject/SEManager/SEManager.h"
#include <cmath>
#include"GameSave/GameSave.h"

namespace {
    constexpr float kBlockWidth = 1040.0f;
    constexpr float kBlockHeight = 240.0f;
    constexpr float kBlockSpacingY = 280.0f;
    constexpr float kScreenWidth = 1280.0f;
    constexpr float kScreenHeight = 720.0f;
    constexpr float kBlockCenterX = kScreenWidth * 0.5f;
    constexpr float kBlockCenterY = kScreenHeight * 0.5f;
    constexpr float kThumbnailMargin = 12.0f;

    float Lerp(float start, float end, float t) { return start + (end - start) * t; }

    Vector2 Lerp(const Vector2& start, const Vector2& end, float t) { return { Lerp(start.x, end.x, t), Lerp(start.y, end.y, t) }; }

    Vector4 Lerp(const Vector4& start, const Vector4& end, float t) { return { Lerp(start.x, end.x, t), Lerp(start.y, end.y, t), Lerp(start.z, end.z, t), Lerp(start.w, end.w, t) }; }
} // namespace



void GameContinued::LoadAllSlots()
{
    auto& gameSave = GameSave::GetInstance();
    for (int i = 0; i < saveDataMaxNum_; ++i) {
        if (gameSave.IsFileExistsAndLoad(i)) {
            gameSave.LoadFromIndex(i);
            auto progressSaveData = gameSave.GetProgressSaveData();
            auto saveDataTime = gameSave.GetSaveDataTime();
            LoadAndSetSpriteHandle(i, false);
            //表示するセーブデータのテキストをセットする
            SetSaveData(i, "SaveFile", progressSaveData.currentStageName.c_str(), saveDataTime.c_str());
        } else {
            SetSaveData(i, "SaveFile", "NoData", "--:--");
        }
    }
}

void GameContinued::LoadAndSetSpriteHandle(const int index, const bool isAllowOverlap)
{
    auto& sprite = gameSaveData_.at(index).GameSceneSprite_;
    auto fileName = GameSave::GetInstance().GetScreenShotFileName(index);
    int screenTextureHandle_ = 0;

    screenTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePathAndRefreshTexture(fileName);


    sprite->SetTextureHandle(screenTextureHandle_);
}

void GameContinued::Initialize() {
    int blockTextureHandle_ = 0;
    int screenTextureHandle_ = 0;
    blockTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");

    for (auto& saveData : gameSaveData_) {
        screenTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/SaveScreenShot/NoData.png");
        saveData.GameSceneSprite_ = std::make_unique<Sprite>();
        saveData.GameSceneSprite_->Initialize(screenTextureHandle_);
        saveData.GameSceneSprite_->SetAnchorPoint({ 0.5f, 0.5f });
        saveData.BlockSprite_ = std::make_unique<Sprite>();
        saveData.BlockSprite_->Initialize(blockTextureHandle_);
        saveData.BlockSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    }
    text_ = std::make_unique<GameContinuedText>();
    text_->Initialize();

    LoadAllSlots();

    for (int i = 0; i < saveDataMaxNum_; ++i) {

        const float y = kBlockCenterY + (kBlockSpacingY * static_cast<float>(i));
        blockPositions_[i] = { kBlockCenterX, y };
        gameSaveData_[i].BlockSprite_->SetPosition(blockPositions_[i]);
        gameSaveData_[i].BlockSprite_->SetScale({ kBlockWidth, kBlockHeight });
        blockColors_[i] = { 0.3f, 0.3f, 0.3f, 0.86f };
        blockScales_[i] = { kBlockWidth, kBlockHeight };
        gameSaveData_[i].BlockSprite_->SetColor(blockColors_[i]);
        gameSaveData_[i].BlockSprite_->Update();

        const float thumbnailHeight = std::max(1.0f, kBlockHeight - (kThumbnailMargin * 2.0f));
        const float thumbnailWidth = thumbnailHeight * (16.0f / 9.0f);
        const float left = blockPositions_[i].x - (kBlockWidth * 0.5f);
        gameSaveData_[i].GameSceneSprite_->SetPosition({ left + kThumbnailMargin + (thumbnailWidth * 0.5f), blockPositions_[i].y });
        gameSaveData_[i].GameSceneSprite_->SetScale({ thumbnailWidth, thumbnailHeight });
        gameSaveData_[i].GameSceneSprite_->SetColor(COLOR::WHITE);
        gameSaveData_[i].GameSceneSprite_->Update();
        text_->SetBlockLayout(i, blockPositions_[i], blockScales_[i]);
    }

    currentSelectNum_ = 0;
    isSelected_ = false;
}


void GameContinued::Update() {
    PlayerCommand* command = PlayerCommand::GetInstance();
    if (command->UiMoveForwardTrigger() || command->MouseWheelDown()) {
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
        if (currentSelectNum_ > 0) {
            --currentSelectNum_;
        }
    }

    if (command->UiMoveBackwardTrigger() || command->MouseWheelUp()) {
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
        if (currentSelectNum_ < saveDataMaxNum_ - 1) {
            ++currentSelectNum_;
        }
    }

    if (command->UiInteractTrigger()) {
        SEManager::SoundPlay(SEManager::PORTAL_SPAWN);
        isSelected_ = true;
    }

    for (int i = 0; i < saveDataMaxNum_; ++i) {
        const float maxDistance = static_cast<float>(saveDataMaxNum_ - 1);
        const float distance = std::abs(static_cast<float>(i - currentSelectNum_));
        const float distanceRate = distance / maxDistance;
        const float easedDistance = distanceRate * distanceRate;
        const float proximity = 1.0f - distanceRate;

        const float targetY = kBlockCenterY + (kBlockSpacingY * (static_cast<float>(i) - static_cast<float>(currentSelectNum_)));
        const Vector2 targetPosition = { kBlockCenterX, targetY };

        Vector4 targetColor{};
        if (i == currentSelectNum_) {
            targetColor = { 0.88f, 0.90f, 0.98f, 0.96f };
        } else {
            const float darken = std::pow(proximity, 1.2f);
            const float r = 0.03f + (darken * 0.10f);
            const float g = 0.04f + (darken * 0.12f);
            const float b = 0.06f + (darken * 0.16f);
            const float alpha = 0.72f + (darken * 0.16f);
            targetColor = { r, g, b, alpha };
        }

        const float scaleFactor = 0.72f + (std::pow(1.0f - easedDistance, 1.4f) * 0.42f);
        const Vector2 targetScale = { kBlockWidth * scaleFactor, kBlockHeight * scaleFactor };

        const float deltaTime = SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        const float transitionRate = std::clamp(deltaTime * 8.0f, 0.0f, 1.0f);
        blockPositions_[i] = Lerp(blockPositions_[i], targetPosition, transitionRate);
        blockColors_[i] = Lerp(blockColors_[i], targetColor, transitionRate);
        blockScales_[i] = Lerp(blockScales_[i], targetScale, transitionRate);

        gameSaveData_[i].BlockSprite_->SetPosition(blockPositions_[i]);
        gameSaveData_[i].BlockSprite_->SetColor(blockColors_[i]);
        gameSaveData_[i].BlockSprite_->SetScale(blockScales_[i]);
        gameSaveData_[i].BlockSprite_->Update();
        const float thumbnailHeight = std::max(1.0f, blockScales_[i].y - (kThumbnailMargin * 2.0f));
        const float thumbnailWidth = thumbnailHeight * (16.0f / 9.0f);
        const float left = blockPositions_[i].x - (blockScales_[i].x * 0.5f);
        gameSaveData_[i].GameSceneSprite_->SetPosition({ left + kThumbnailMargin + (thumbnailWidth * 0.5f), blockPositions_[i].y });
        gameSaveData_[i].GameSceneSprite_->SetScale({ thumbnailWidth, thumbnailHeight });
        gameSaveData_[i].GameSceneSprite_->Update();
        text_->SetBlockLayout(i, blockPositions_[i], blockScales_[i]);
    }

    text_->Update(currentSelectNum_);
}

void GameContinued::Draw() {
    SpriteCommon::GetInstance()->DrawCommon();
    for (auto& saveData : gameSaveData_) {
        saveData.BlockSprite_->Draw();
        saveData.GameSceneSprite_->Draw();
    }
    SpriteCommon::GetInstance()->DrawCommonFont();
    text_->Draw();
}

void GameContinued::SetSaveData(int index, const std::string& name, const std::string& currentStageName, const std::string& saveDateTime) {
    if (index < 0 || index >= saveDataMaxNum_) {
        return;
    }

    gameSaveData_[index].Name_ = name;
    gameSaveData_[index].currentStageName_ = currentStageName;
    gameSaveData_[index].saveDateTime_ = saveDateTime;

    if (text_) {
        text_->SetSaveDataText(name, currentStageName, saveDateTime, index);
    }
}