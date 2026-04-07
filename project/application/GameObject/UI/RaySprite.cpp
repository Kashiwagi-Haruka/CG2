#include "RaySprite.h"
#include "TextureManager.h"
#include "Sprite/SpriteCommon.h"
#include"WinApp.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Portal/PortalManager.h"
#include"GameObject/Player/Player.h"
#include"ScreenSize/ScreenSize.h"
#include"GameObject/Box/BoxManager.h"
#include"GameObject/Chair/ChairManager.h"
#include"GameObject/Desk/DeskManager.h"
#include"GameObject/Door/Door.h"
#include"GameObject/Edamame/Edamame.h"
#include"GameObject/Flashlight/Flashlight.h"
#include"GameObject/Key/Key.h"
#include"GameObject/Elevator/Elevator.h"
#include"GameObject/Locker/LockerManager.h"
#include"GameObject/VendingMac/VendingMac.h"
#include"GameObject/PC/PC.h"
#include"GameObject/GentleMan/GentleMan.h"
RaySprite::RaySprite()
{
    handHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/hand.png");
    grabHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/grabHand.png");
    portalHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/mouseUI.png");

    sprite_ = std::make_unique<Sprite>();

}

void RaySprite::Initialize()
{
    sprite_->Initialize(handHandle_);
    sprite_->SetAnchorPoint({ 0.5f,0.5f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ 128.0f,128.0f });
    sprite_->SetPosition({ SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HALF_HEIGHT });
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();
}

void RaySprite::Update()
{
    auto* playerCommand = PlayerCommand::GetInstance();

    if (PlayerCommand::GetIsGrab()) {
        // インタラクトの処理
        SetTexture(RaySprite::GRAB);
    } else if (PortalManager::GetCanMakePortal()) {
        SetTexture(RaySprite::PORTAL);
    } else {
        SetTexture(RaySprite::HAND);
    }

    sprite_->Update();
}

void RaySprite::Draw()
{
    SpriteCommon::GetInstance()->DrawCommon();

    if (IsRayHit() || PlayerCommand::GetIsGrab() || PortalManager::GetCanMakePortal()|| Gentleman::IsRayHit()) {
        sprite_->Draw();
    }

}

void RaySprite::SetTexture(const TextureUI num)
{
    switch (num)
    {
    case TextureUI::PORTAL:
        sprite_->SetTextureHandle(portalHandle_);
        break;
    case TextureUI::HAND:
        sprite_->SetTextureHandle(handHandle_);
        break;
    case TextureUI::GRAB:
        sprite_->SetTextureHandle(grabHandle_);
        break;
    default:
        break;
    }
}

bool RaySprite::IsRayHit()
{
    return
        BoxManager::IsRayHit() ||
        ChairManager::IsRayHit() ||
        DeskManager::IsRayHit() ||
        LockerManager::IsRayHit() ||
        Door::IsRayHit() ||
        Edamame::IsRayHit() ||
        Flashlight::IsRayHit() ||
        Key::IsRayHit() ||
        VendingMac::IsRayHit() ||
        PC::IsRayHit();
}
