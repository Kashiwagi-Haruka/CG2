#include "KeyIcon.h"
#include "TabKey.h"
#include "TextureManager.h"
#include"ScreenSize/ScreenSize.h"

KeyIcon::KeyIcon()
{
    handle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/keyIcon.png");
    sprite_ = std::make_unique<Sprite>();
}

void KeyIcon::Initialize()
{
    sprite_->Initialize(handle_);
    sprite_->SetAnchorPoint({ 0.0f,1.0f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ 128.0f,128.0f });
    sprite_->SetPosition({ 64.0f+128.0f, SCREEN_SIZE::HEIGHT - 32.0f });
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();
}

void KeyIcon::Update()
{
    sprite_->Update();
}

void KeyIcon::Draw()
{
    sprite_->Draw();
}
