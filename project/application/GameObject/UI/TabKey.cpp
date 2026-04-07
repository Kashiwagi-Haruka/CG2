#include "TabKey.h"
#include "TextureManager.h"
#include"ScreenSize/ScreenSize.h"
#include"SpriteCommon.h"

TabKey::TabKey()
{
    handle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/tab.png");
    sprite_ = std::make_unique<Sprite>();
}

void TabKey::Initialize()
{
    sprite_->Initialize(handle_);
    sprite_->SetAnchorPoint({ 0.0f,0.0f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ 224.0f,64.0f });
    sprite_->SetPosition({ 32.0f,32.0f});
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();
}

void TabKey::Update()
{
    sprite_->Update();
}

void TabKey::Draw()
{
    sprite_->Draw();
}
