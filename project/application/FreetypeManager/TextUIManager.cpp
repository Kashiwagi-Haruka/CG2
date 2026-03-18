#include "TextUIManager.h"
#include"SpriteCommon.h"
#include"GameObject/Door/Door.h"
#include"DirectXCommon.h"
#include"GameObject/Edamame/EdamameTrivia.h"
#include <codecvt>
#include <locale>

TextUIManager::TextUIManager()
{
    /// @brief ライブラリの初期化
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
    edamameTrivia_.SetPosition({640,512+64 });
    edamameTrivia_.SetColor({ 1, 1, 1, 1 });
    edamameTrivia_.SetAlign(TextAlign::Center);
    edamameTrivia_.SetBlendMode(BlendMode::kBlendModeAlpha);
    edamameTrivia_.UpdateLayout();

}

TextUIManager::~TextUIManager()
{
    /// @brief 終了処理
    FreeTypeManager::Finalize();
}

void TextUIManager::Initialize()
{
    isDraw_ = false;
    showTimer_ = showTime_;
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

    if (Door::GetLockMassage()) {
        text_.SetString(U"鍵がかかっている。");
        text_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        isDraw_ = true;
    }

    if (Door::GetOpenMassage()) {
        text_.SetString(U"扉が開いた。");
        text_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
        isDraw_ = true;
    }

    if (EdamameTrivia::GetIsSendStartTriviaMessage()) {
        edamameTrivia_.SetString(EdamameTrivia::GetString());
        edamameTrivia_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
    }

    text_.Update();
    edamameTrivia_.Update();
}

void TextUIManager::Draw()
{
    SpriteCommon::GetInstance()->DrawCommonFont();
    edamameTrivia_.Draw();
    FreeTypeManager::ResetFontUsage();

    if (!isDraw_) { return; }
    SpriteCommon::GetInstance()->DrawCommonFont();
    text_.Draw();
    FreeTypeManager::ResetFontUsage();

   
}
