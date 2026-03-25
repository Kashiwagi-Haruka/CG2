#include "TextUIManager.h"
#include"SpriteCommon.h"
#include"GameObject/Door/Door.h"
#include"DirectXCommon.h"
#include"GameObject/Edamame/EdamameTrivia.h"
#include"GameObject/Key/Key.h"
#include <codecvt>
#include <locale>

TextUIManager::TextUIManager()
{

    FreeTypeManager::Initialize();
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 24, 24);
    text_.Initialize(fontHandle_);
    text_.SetString(U"p-再打刻");
    text_.SetPosition({ 640,240 });
    text_.SetColor({ 1, 1, 1, 1 });
    text_.SetAlign(TextAlign::Center);
    text_.SetBlendMode(BlendMode::kBlendModeAlpha);
    text_.UpdateLayout();

    edamameTrivia_.Initialize(fontHandle_);
    edamameTrivia_.SetPosition({ 640,512 + 64 });
    edamameTrivia_.SetColor({ 1, 1, 1, 1 });
    edamameTrivia_.SetAlign(TextAlign::Center);
    edamameTrivia_.SetBlendMode(BlendMode::kBlendModeAlpha);
    edamameTrivia_.UpdateLayout();

    chairMenu_ = std::make_unique<ChairMenu>();
}

TextUIManager::~TextUIManager()
{

}

void TextUIManager::Initialize()
{
    isDraw_ = false;
    showTimer_ = showTime_;
    chairMenu_->Initialize();
}

void TextUIManager::Update()
{
    if (isDraw_) {
        const float deltaTime = SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        showTimer_ -= deltaTime;
        if (showTimer_ <= 0.0f) {
            showTimer_ = showTime_;
            isDraw_ = false;
        }
    }


    if (Door::GetOpenMassage()) {
        text_.SetString(U"扉が開いた。");
        text_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        isDraw_ = true;
    } else if (Key::GetGetKeyMessage()) {
        text_.SetString(U"鍵を入手した。");
        text_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        isDraw_ = true;
    } else if (Door::GetLockMassage()) {
        text_.SetString(U"鍵がかかっている。");
        text_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        isDraw_ = true;
    }


    if (EdamameTrivia::GetIsSendStartTriviaMessage()) {
        edamameTrivia_.SetString(EdamameTrivia::GetString());
        edamameTrivia_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
    }

    text_.Update();
    edamameTrivia_.Update();

    chairMenu_->Update();
}

void TextUIManager::Draw()
{

    edamameTrivia_.Draw();

    if (isDraw_) {
        text_.Draw();
    }

    chairMenu_->Draw();

    FreeTypeManager::ResetFontUsage();
}
