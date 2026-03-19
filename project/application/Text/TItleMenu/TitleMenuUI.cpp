#include "TitleMenuUI.h"
#include"WinApp.h"
#include"GameObject/KeyBindConfig.h"
#include"DirectXCommon.h"
#include "Sprite/SpriteCommon.h"
#include"ScreenSize/ScreenSize.h"
#include"Color/Color.h"

TitleMenuUI::TitleMenuUI()
{
    //SEの初期化
    SEData_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/pushWatch.mp3");
    Audio::GetInstance()->SetSoundVolume(&SEData_, 1.0f);


    //フォントハンドル
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 64, 64);

    random_ = std::make_unique<RandomClass>();
    titleDefaultPos_ = { SCREEN_SIZE::HALF_WIDTH,240 };

    titleText_.Initialize(fontHandle_);
    titleText_.SetString(U"p-再打刻");
    titleText_.SetPosition(titleDefaultPos_);
    titleText_.SetColor(COLOR::WHITE);
    titleText_.SetAlign(TextAlign::Center);
    titleText_.SetBlendMode(BlendMode::kBlendModeAlpha);

    menuFontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(menuFontHandle_, 32, 32);
    for (auto& text : menuText_) {
        text.Initialize(menuFontHandle_);
        text.SetColor(COLOR::WHITE);
        text.SetAlign(TextAlign::Left);
    }

    menuText_[START_TEXT].SetString(U"出勤する");
    menuText_[START_TEXT].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 256.0f ,SCREEN_SIZE::HALF_HEIGHT });

    menuText_[CONTINUE_TEXT].SetString(U"再打刻する");
    menuText_[CONTINUE_TEXT].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 256.0f,SCREEN_SIZE::HALF_HEIGHT + 64.0f });

    menuText_[OPTION_TEXT].SetString(U"オプション");
    menuText_[OPTION_TEXT].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 256.0f,SCREEN_SIZE::HALF_HEIGHT + 128.0f });

    triangleText_.Initialize(menuFontHandle_);
    triangleText_.SetString(U"▶");
    Vector2 pos = menuText_[selectButtonNum_].GetPosition();
    pos.x -= 32.0f;
    triangleText_.SetPosition(pos);
    triangleText_.SetColor(COLOR::RED);
    triangleText_.SetAlign(TextAlign::Left);
    triangleText_.SetBlendMode(BlendMode::kBlendModeAlpha);

    fontTheta_ = 0.0f;

    pressSpaceText_.Initialize(menuFontHandle_);
    pressSpaceText_.SetString(U"スペースor左クリック");
    pressSpaceText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HEIGHT - 128.0f });
    pressSpaceText_.SetColor(COLOR::WHITE);
    pressSpaceText_.SetAlign(TextAlign::Center);
    pressSpaceText_.SetBlendMode(BlendMode::kBlendModeAlpha);
}

TitleMenuUI::~TitleMenuUI()
{
    Audio::GetInstance()->SoundUnload(&SEData_);
}

void TitleMenuUI::Initialize()
{
    isShowMenu_ = false;
    isSelectButton_ = false;
    isStart_ = false;
    selectButtonNum_ = 0;
    random_->SetMinMax(-8.0f, 8.0f);
}

void TitleMenuUI::Update()
{

    fontTheta_ += SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime() * Function::kPi;
    fontTheta_ = fmodf(fontTheta_, Function::kPi * 2.0f);
    float fontAlpha = std::sinf(fontTheta_) * 0.5f + 0.5f;

    pressSpaceText_.SetColor({ 1.0f,1.0f,1.0f,fontAlpha });
    pressSpaceText_.UpdateLayout(false);

    if (isStart_) {
        return;
    }

    if (PlayerCommand::GetInstance()->Shot()) {
        Audio::GetInstance()->SoundPlayWave(SEData_, false);
        if (isShowMenu_) {
            if (selectButtonNum_ == 0) {
                if (!isStart_) {
                    isStart_ = true;
                }
            }

        } else {
            isShowMenu_ = true;
            for (auto& text : menuText_) {
                text.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
            }
        }
    }

    if (isShowMenu_) {

        if (PlayerCommand::GetInstance()->MoveForwardTrigger()) {
            if (selectButtonNum_ > 0) {
                Audio::GetInstance()->SoundPlayWave(SEData_, false);
                selectButtonNum_--;
            }
            menuText_[selectButtonNum_].StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        }

        if (PlayerCommand::GetInstance()->MoveBackwardTrigger()) {
            if (selectButtonNum_ < menuText_.size() - 1) {
                Audio::GetInstance()->SoundPlayWave(SEData_, false);
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
    }



    if (rand() % 60 == 0) {
        Vector2 pos = titleDefaultPos_;
        pos.x += random_->Get();
        pos.y += random_->Get();
        titleText_.SetPosition(pos);
    } else {
        titleText_.SetPosition(titleDefaultPos_);
    }
    titleText_.UpdateLayout(false);


}

void TitleMenuUI::Draw()
{
    pressSpaceText_.Draw();

    if (isStart_) { return; }
    //スタートしてないとき
    titleText_.Draw();

    if (isShowMenu_) {
        triangleText_.Draw();
        for (auto& text : menuText_) {
            text.Draw();
        }
    }

}

