#include "HintSheetManager.h"
#include"ScreenSize.h"
#include"TextureManager.h"
#include"SpriteCommon.h"

void HintSheetManager::Initialize() {
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 32, 32);
    hintSheets_.clear();
    uint32_t handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/white2x2.png");
    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize(handle);
    sprite_->SetAnchorPoint({ 0.5f,0.5f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ SCREEN_SIZE::WIDTH,SCREEN_SIZE::HEIGHT });
    sprite_->SetPosition({ SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HALF_HEIGHT });
    sprite_->SetColor({ 0.0f,0.0f,0.0f,0.5f });
    sprite_->Update();
}

void HintSheetManager::AddHintSheet(const std::string& textFilePath) {
    auto newSheet = std::make_unique<HintSheet>();

    // 初期化（フォントとファイルパスを渡す）
    newSheet->Initialize(fontHandle_, textFilePath);

    // すでにカメラが設定されている場合は適用する
    if (camera_) {
        newSheet->SetCamera(camera_);
    }

    hintSheets_.push_back(std::move(newSheet));
}

void HintSheetManager::Update() {
    for (auto& sheet : hintSheets_) {
        sheet->Update();
    }
}

void HintSheetManager::Draw() {
    for (auto& sheet : hintSheets_) {
        sheet->Draw();
    }
}

void HintSheetManager::DrawUI() {

    for (auto& sheet : hintSheets_) {
        if (sheet->GetIsLooking()) {
            SpriteCommon::GetInstance()->DrawCommon();
            sprite_->Draw();
            break;
        }
    }

    for (auto& sheet : hintSheets_) {
        // 現在「見ている」状態のシートだけが内部でDrawを実行します
        sheet->DrawUI();
    }
}

void HintSheetManager::SetCamera(Camera* camera) {
    camera_ = camera;
    for (auto& sheet : hintSheets_) {
        sheet->SetCamera(camera);
    }
}

void HintSheetManager::SetPlayerCamera(PlayerCamera* playerCamera) {
    HintSheet::SetPlayerCamera(playerCamera);
}

void HintSheetManager::SetParentMatrix(const uint32_t index, Matrix4x4* parentMatrix)
{
    assert(index < hintSheets_.size());
    hintSheets_.at(index)->SetParentMatrix(parentMatrix);
}

void HintSheetManager::SetParentIsOpenEnd(const uint32_t index, bool* isOpenEnd)
{
    assert(index < hintSheets_.size());
    hintSheets_.at(index)->SetOpenAnimationEndPtr(isOpenEnd);
}
