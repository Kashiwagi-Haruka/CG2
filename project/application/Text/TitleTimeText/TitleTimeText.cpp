#include "TitleTimeText.h"

#include"ScreenSize/ScreenSize.h"
#include"Color/Color.h"
#include"GameObject/SEManager/SEManager.h"
#include"GameSave/GameSave.h"

TitleTimeText::TitleTimeText()
{
    //フォントハンドル
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 32, 32);

    dateText_.Initialize(fontHandle_);
    dateText_.SetString(U"test");
    dateText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH + 64.0f,SCREEN_SIZE::HALF_HEIGHT });
    dateText_.SetColor(COLOR::WHITE);
    dateText_.SetAlign(TextAlign::Center);
    dateText_.SetBlendMode(BlendMode::kBlendModeAdd);
}

void TitleTimeText::Initialize()
{
    preCurrentDataTime_ = GameSave::GetInstance().GetCurrentDateTimeString();
}

void TitleTimeText::Update()
{
    std::string saveDateTime = GameSave::GetInstance().GetCurrentDateTimeString();
    if (preCurrentDataTime_ != saveDateTime) {
        preCurrentDataTime_ = saveDateTime;
        SEManager::SoundPlay(SEManager::CLOCK);
        dateText_.SetString(std::u32string(preCurrentDataTime_.begin(), preCurrentDataTime_.end()));
        dateText_.UpdateLayout(false);
    }
}

void TitleTimeText::Draw()
{
    dateText_.Draw();
}
