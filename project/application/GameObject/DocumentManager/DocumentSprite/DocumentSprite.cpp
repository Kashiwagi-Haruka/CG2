#include "DocumentSprite.h"
#include "TextureManager.h"
#include "Sprite/SpriteCommon.h"
#include"ScreenSize/ScreenSize.h"
#include"GameObject/KeyBindConfig.h"
#include<algorithm>
DocumentSprite::DocumentSprite()
{
    //handle_[DOC_TIMECARD_WATCH] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/3d/document/document0.png");
    //handle_[DOC_SECRET] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/3d/document/document1.png");

    sprite_ = std::make_unique<Sprite>();

    // スクロールバー用の白テクスチャなどを読み込む（パスはプロジェクト環境に合わせて変更してください）
    scrollbarTexHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/white2x2.png");

    sprite_ = std::make_unique<Sprite>();
    scrollbarTrackSprite_ = std::make_unique<Sprite>();
    scrollbarThumbSprite_ = std::make_unique<Sprite>();

}

void DocumentSprite::Initialize(const std::string name)
{
    std::string filePath = "Resources/TD3_3102/3d/document/" + name + ".png";
    uint32_t handle = TextureManager::GetInstance()->GetTextureIndexByfilePath(filePath);
    sprite_->Initialize(handle);
    sprite_->SetAnchorPoint({ 0.5f,0.0f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ 725.0f,1024.0f });
    position_ = { SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HALF_HEIGHT };
    sprite_->SetPosition(position_);
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();

    // スクロールバー（背景）の初期化
    scrollbarTrackSprite_->Initialize(scrollbarTexHandle_);
    scrollbarTrackSprite_->SetAnchorPoint({ 0.5f, 0.0f });
    // 画面右端から少し左にずらした位置。上下の余白は100とする
    scrollbarTrackSprite_->SetPosition({ SCREEN_SIZE::WIDTH - 40.0f, 100.0f });
    scrollbarTrackSprite_->SetScale({ 8.0f, SCREEN_SIZE::HEIGHT - 200.0f });
    scrollbarTrackSprite_->SetColor({ 0.2f, 0.2f, 0.2f, 0.5f }); // 半透明の暗いグレー
    scrollbarTrackSprite_->Update();

    // スクロールバー（つまみ）の初期化
    scrollbarThumbSprite_->Initialize(scrollbarTexHandle_);
    scrollbarThumbSprite_->SetAnchorPoint({ 0.5f, 0.0f });
    scrollbarThumbSprite_->SetScale({ 8.0f, 100.0f }); // つまみの長さは100
    scrollbarThumbSprite_->SetColor({ 0.8f, 0.8f, 0.8f, 1.0f }); // 明るいグレー
    scrollbarThumbSprite_->Update();
}

void DocumentSprite::Update()
{
    auto* playerCommand = PlayerCommand::GetInstance();
    // 1. 書類のY座標更新
    position_.y += playerCommand->GetMouseWheelDelta();

    // 書類の移動限界値
    float minY = -(sprite_->GetScale().y + SCREEN_SIZE::HALF_HEIGHT) * 0.5f;
    float maxY = SCREEN_SIZE::HALF_HEIGHT;


    position_.y = std::clamp(position_.y, minY, maxY);
    sprite_->SetPosition(position_);
    sprite_->Update();


    // 2. スクロールバーつまみのY座標更新
    float scrollRatio = 0.0f;
    if (maxY - minY != 0.0f) {
        // maxY の時が一番上(0.0)、minY の時が一番下(1.0)になるように割合を計算
        scrollRatio = (maxY - position_.y) / (maxY - minY);
    }

    // つまみの移動可能範囲
    float trackY = scrollbarTrackSprite_->GetTranslate().y;
    float trackHeight = scrollbarTrackSprite_->GetScale().y;
    float thumbHeight = scrollbarThumbSprite_->GetScale().y;

    // トラック内でつまみが動くY座標を計算
    float thumbY = trackY + (trackHeight - thumbHeight) * scrollRatio;

    // つまみの位置を更新
    scrollbarThumbSprite_->SetPosition({ SCREEN_SIZE::WIDTH - 40.0f, thumbY });

    scrollbarTrackSprite_->Update();
    scrollbarThumbSprite_->Update();
}

void DocumentSprite::Draw()
{
    SpriteCommon::GetInstance()->DrawCommon();
    sprite_->Draw();

    // スクロールバーを描画
    scrollbarTrackSprite_->Draw();
    scrollbarThumbSprite_->Draw();
}
