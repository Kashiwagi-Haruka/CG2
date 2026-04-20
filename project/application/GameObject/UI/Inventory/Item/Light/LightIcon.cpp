#include "LightIcon.h"
#include "TextureManager.h"
#include"ScreenSize/ScreenSize.h"

LightIcon::LightIcon()
{
    handle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/flashLightIcon.png");
    sprite_ = std::make_unique<Sprite>();
}

void LightIcon::Initialize()
{

    sprite_->Initialize(handle_);
    sprite_->SetAnchorPoint({ 0.0f,1.0f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ 128.0f,128.0f });
    sprite_->SetPosition({ 32.0f, SCREEN_SIZE::HEIGHT - 32.0f });
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();
}

void LightIcon::Update()
{
    sprite_->Update();
}

void LightIcon::Draw()
{
    sprite_->Draw();
}
