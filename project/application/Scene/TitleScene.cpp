#include "TitleScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <imgui.h>
#include"FreetypeManager/FreeTypeManager.h"
#include"GameObject/KeyBindConfig.h"
#include"WinApp.h"
#include"DirectXCommon.h"
#include"Function.h"

namespace COLOR {
    constexpr Vector4 RED = { 1.0f,0.0f,0.0f,1.0f };
    constexpr Vector4 WHITE = { 1.0f,1.0f,1.0f,1.0f };
}

namespace SCREEN_SIZE {
    const float WIDTH = static_cast<float>(WinApp::kClientWidth);
    const float HEIGHT = static_cast<float>(WinApp::kClientHeight);
    const float HALF_WIDTH = WIDTH * 0.5f;
    const float HALF_HEIGHT = HEIGHT * 0.5f;
}

TitleScene::TitleScene() {

    FirstSettingText();

    BGMData_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/clock.mp3");
    Audio::GetInstance()->SetSoundVolume(&BGMData_, 1.0f);

    SEData_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/pushWatch.mp3");
    Audio::GetInstance()->SetSoundVolume(&SEData_, 1.0f);

    transition = std::make_unique<SceneTransition>();
}

void TitleScene::Finalize() {
    Audio::GetInstance()->SoundUnload(&BGMData_);
    Audio::GetInstance()->SoundUnload(&SEData_);

}

void TitleScene::FirstSettingText()
{
    /// @brief 初期化
    FreeTypeManager::Initialize();
    //フォントハンドル
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 64, 64);
    titleText_.Initialize(fontHandle_);
    titleText_.SetString(U"p-再打刻");
    titleText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH,240 });
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
    menuText_[START_TEXT].SetPosition({ SCREEN_SIZE::HALF_WIDTH+128.0f ,SCREEN_SIZE::HALF_HEIGHT});

    menuText_[CONTINUE_TEXT].SetString(U"再打刻する");
    menuText_[CONTINUE_TEXT].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 128.0f,SCREEN_SIZE::HALF_HEIGHT + 64.0f });

    menuText_[OPTION_TEXT].SetString(U"オプション");
    menuText_[OPTION_TEXT].SetPosition({ SCREEN_SIZE::HALF_WIDTH + 128.0f,SCREEN_SIZE::HALF_HEIGHT + 128.0f });

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
    pressSpaceText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HEIGHT-128.0f });
    pressSpaceText_.SetColor(COLOR::WHITE);
    pressSpaceText_.SetAlign(TextAlign::Center);
    pressSpaceText_.SetBlendMode(BlendMode::kBlendModeAlpha);
}


void TitleScene::Initialize() {

    isBGMPlaying = false;
    isTransitionIn = true;
    isTransitionOut = false;
    isShowMenu_ = false;
    isSelectButton_ = false;

    transition->Initialize(false);
    titleText_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
    selectButtonNum_ = 0;
}

void TitleScene::Update() {
    if (!isBGMPlaying) {
        Audio::GetInstance()->SoundPlayWave(BGMData_, true);
        isBGMPlaying = true;
    }


    if (PlayerCommand::GetInstance()->Shot()) {
        Audio::GetInstance()->SoundPlayWave(SEData_, false);
        if (isShowMenu_) {
            if (selectButtonNum_ == 0) {
                if (!isTransitionOut) {
                    transition->Initialize(true);
                    isTransitionOut = true;
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

    titleText_.Update();
    
    fontTheta_ += SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime()*Function::kPi;
    fontTheta_ = fmodf(fontTheta_, Function::kPi * 2.0f);
    float fontAlpha = std::sinf(fontTheta_)*0.5f+0.5f;

#ifdef USE_IMGUI

    ImGui::Begin("FontTheta");
    ImGui::Text("fontAlpha:%f", fontAlpha);
    ImGui::Text("fontTheta:%f", fontTheta_);
    ImGui::End();
#endif

    pressSpaceText_.SetColor({ 1.0f,1.0f,1.0f,fontAlpha });
    pressSpaceText_.UpdateLayout(false);

    if (isTransitionIn || isTransitionOut) {
        transition->Update();
        if (transition->IsEnd() && isTransitionIn) {
            isTransitionIn = false;
        }
        if (transition->IsEnd() && isTransitionOut) {
            SceneManager::GetInstance()->ChangeScene("ShadowGame");
        }
    }
}
void TitleScene::Draw() {

    SpriteCommon::GetInstance()->DrawCommon();

    if (isTransitionIn || isTransitionOut) {
        transition->Draw();
    }

    SpriteCommon::GetInstance()->DrawCommonFont();
    titleText_.Draw();
  
    if (isShowMenu_) {
        triangleText_.Draw();
        for (auto& text : menuText_) {
            text.Draw();
        }
    }
    pressSpaceText_.Draw();

    FreeTypeManager::ResetFontUsage();

}