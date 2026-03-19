#include "FirstStory.h"
#include"Text/FreetypeManager/FreeTypeManager.h"
#include"ScreenSize/ScreenSize.h"
#include"Color/Color.h"
#include"SpriteCommon.h"
#include"GameObject/KeyBindConfig.h"

FirstStory::FirstStory()
{
    //フォントハンドル
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 24, 24);

    text_.Initialize(fontHandle_);
    text_.SetPosition({ SCREEN_SIZE::HALF_WIDTH,128.0f+64.0f });
    text_.SetColor(COLOR::WHITE);
    text_.SetAlign(TextAlign::Center);
    text_.SetBlendMode(BlendMode::kBlendModeAlpha);

    strings_ = U"その日も残業だった。\n気が付くと他の部署はとっくに消灯していて、\n閑散とした部屋にパソコンのモーター音だけが\nうるさく鳴り響いていた。\n眠気覚ましにコーヒーサーバーを動かす。\n湯気は立っているのに、香りがほとんどしない。\n味も薄い。機械の調子が悪いのか、\nそれとも自分の舌が麻痺しているのか。\n「残りは家でやるか。」\nそうして、いつものように打刻機へ手を伸ばした。";
}

void FirstStory::Initialize()
{
    shotCount_ = 0;
    isEnd_ = false;
}

void FirstStory::Update()
{

    if (PlayerCommand::GetInstance()->Shot()) {
        if (shotCount_ < 1) {
            text_.SetString(strings_);
            text_.StartTyping(0.05f);
        }
        shotCount_++;
    }

    if (shotCount_ <= 1) {
        text_.Update(true);
    } else {
        text_.UpdateLayout(false);
        isEnd_ = true;
    }
}

void FirstStory::Draw()
{

    text_.Draw();
    //FreeTypeManager::ResetFontUsage();
}
