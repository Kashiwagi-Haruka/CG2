#include "GentlemanMenu.h"
#include"GameObject/KeyBindConfig.h"
#include"DirectXCommon.h"
#include "Sprite/SpriteCommon.h"
#include"ScreenSize/ScreenSize.h"
#include"Color/Color.h"
#include"GameObject/SEManager/SEManager.h"

 bool GentlemanMenu::isShowMenu_ = false;
 uint32_t GentlemanMenu::selectButtonNum_ = GentlemanMenu::TALK;

GentlemanMenu::GentlemanMenu()
{
    menuFontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(menuFontHandle_, 32, 32);
    for (auto& text : menuText_) {
        text.Initialize(menuFontHandle_);
        text.SetColor(COLOR::WHITE);
        text.SetAlign(TextAlign::Left);
    }

    menuText_[TALK].SetString(U"話す");
    menuText_[TALK].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 256.0f ,SCREEN_SIZE::HALF_HEIGHT });

    menuText_[SAVE].SetString(U"セーブする");
    menuText_[SAVE].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 256.0f,SCREEN_SIZE::HALF_HEIGHT + 64.0f });

    menuText_[CANCEL].SetString(U"閉じる");
    menuText_[CANCEL].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 256.0f,SCREEN_SIZE::HALF_HEIGHT + 128.0f });

    triangleText_.Initialize(menuFontHandle_);
    triangleText_.SetString(U"▶");
    Vector2 pos = menuText_[selectButtonNum_].GetPosition();
    pos.x -= 32.0f;

    triangleText_.SetPosition(pos);
    triangleText_.SetColor(COLOR::RED);
    triangleText_.SetAlign(TextAlign::Left);
    triangleText_.SetBlendMode(BlendMode::kBlendModeAlpha);

    fontTheta_ = 0.0f;

    pressEText_.Initialize(menuFontHandle_);
    pressEText_.SetString(U"マウスホイールで選択");
    pressEText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HEIGHT - 128.0f });
    pressEText_.SetColor(COLOR::WHITE);
    pressEText_.SetAlign(TextAlign::Center);
    pressEText_.SetBlendMode(BlendMode::kBlendModeAlpha);
}

GentlemanMenu::~GentlemanMenu()
{
}

void GentlemanMenu::Initialize()
{
    isShowMenu_ = false;
    selectButtonNum_ = 0;
    isShowStart_ = false;
}

void GentlemanMenu::Update()
{

    fontTheta_ += SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime() * Function::kPi;
    fontTheta_ = fmodf(fontTheta_, Function::kPi * 2.0f);
    float fontAlpha = std::sinf(fontTheta_) * 0.5f + 0.5f;

    pressEText_.SetColor({ 1.0f,1.0f,1.0f,fontAlpha });
    pressEText_.UpdateLayout(false);

    if (isShowMenu_) {

        if (!isShowStart_) {
            SEManager::SoundPlay(SEManager::PUSH_WATCH);
            //最初は0番に設定する
            selectButtonNum_ = 0;
            for (auto& text : menuText_) {
                text.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
            }
            isShowStart_ = true;
        }

        //メニューが開いた時
        if (PlayerCommand::GetInstance()->MouseWheelDown()) {
            //ホイールを下にすると
            if (selectButtonNum_ > 0) {
                SEManager::SoundPlay(SEManager::PUSH_WATCH);
                selectButtonNum_--;
            }
            menuText_[selectButtonNum_].StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        }

        if (PlayerCommand::GetInstance()->MouseWheelUp()) {
            //ホイールを上にすると
            if (selectButtonNum_ < menuText_.size() - 1) {
                SEManager::SoundPlay(SEManager::PUSH_WATCH);
                selectButtonNum_++;

            }
            menuText_[selectButtonNum_].StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        }

        for (auto& text : menuText_) {
            text.SetColor(COLOR::WHITE);
        }

        if (selectButtonNum_ < menuText_.size() && selectButtonNum_ >= 0) {
            menuText_[selectButtonNum_].SetColor(Vector4{ COLOR::RED });
            Vector2 pos = menuText_[selectButtonNum_].GetPosition();
            pos.x -= 48.0f;
            triangleText_.SetPosition(pos);
        }

        for (auto& text : menuText_) {
            text.Update();
        }

        triangleText_.UpdateLayout(false);
    } else {
        isShowStart_ = false;
    }

}

void GentlemanMenu::Draw()
{
    if (!isShowMenu_) {
        return;
    }

    triangleText_.Draw();
    pressEText_.Draw();

    for (auto& text : menuText_) {
        text.Draw();
    }

}

