#include "LockerMenu.h"
#include"GameObject/KeyBindConfig.h"
#include"DirectXCommon.h"
#include "Sprite/SpriteCommon.h"
#include"ScreenSize/ScreenSize.h"
#include"Color/Color.h"
#include"GameObject/SEManager/SEManager.h"

bool LockerMenu::isShowMenu_ = false;

LockerMenu::LockerMenu()
{
    menuFontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(menuFontHandle_, 32, 32);

    exitText_.Initialize(menuFontHandle_);
    exitText_.SetColor(COLOR::RED);
    exitText_.SetAlign(TextAlign::Left);
    exitText_.SetString(U"Eで出る");
    exitText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH + 128.0f ,SCREEN_SIZE::HALF_HEIGHT });

    triangleText_.Initialize(menuFontHandle_);
    triangleText_.SetString(U"▶");
    Vector2 pos = exitText_.GetPosition();
    pos.x -= 48.0f;

    triangleText_.SetPosition(pos);
    triangleText_.SetColor(COLOR::RED);
    triangleText_.SetAlign(TextAlign::Left);
    triangleText_.SetBlendMode(BlendMode::kBlendModeAlpha);

    fontTheta_ = 0.0f;

}

LockerMenu::~LockerMenu()
{
}

void LockerMenu::Initialize()
{
    isShowMenu_ = false;
    isShowStart_ = false;
}

void LockerMenu::Update()
{
    if (!isShowMenu_) {
        isShowStart_ = false;
        return;
    }

    //メニューが開いた時や入力移動を検知したとき
    if (!isShowStart_ || PlayerCommand::GetInstance()->MouseWheelDown() || PlayerCommand::GetInstance()->MouseWheelUp()) {
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
        exitText_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        isShowStart_ = true;
    }

    exitText_.Update();

    fontTheta_ += SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime() * Function::kPi;
    fontTheta_ = fmodf(fontTheta_, Function::kPi * 2.0f);
    float fontAlpha = std::sinf(fontTheta_) * 0.5f + 0.5f;

    triangleText_.SetColor({ 1.0f,1.0f,1.0f,fontAlpha });
    triangleText_.UpdateLayout(false);
}

void LockerMenu::Draw()
{

    if (!isShowMenu_) {
        return;
    }

    triangleText_.Draw();
    exitText_.Draw();
}

