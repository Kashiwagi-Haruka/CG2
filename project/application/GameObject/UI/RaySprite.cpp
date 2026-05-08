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
#include"GameObject/DocumentManager/Document/Document.h"
#include"GameObject/GentleMan/GiantGentleMan.h"
RaySprite::RaySprite()
{
    handle_[HAND] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/hand.png");
    handle_[GRAB] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/grabHand.png");
    handle_[PORTAL] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/mouseUI.png");
    handle_[TALK] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/talk.png");

    sprite_ = std::make_unique<Sprite>();

}

void RaySprite::Initialize()
{
    sprite_->Initialize(handle_[HAND]);
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
    } else if (PortalManager::GetCanMakePortal()|| GiantGentleMan::IsRayHit()) {
        SetTexture(RaySprite::PORTAL);
    } else if(Gentleman::IsRayHit()){
        SetTexture(RaySprite::TALK);
    } else {
        SetTexture(RaySprite::HAND);
    }

    sprite_->Update();
}

void RaySprite::Draw()
{
    SpriteCommon::GetInstance()->DrawCommon();

    if (IsRayHit() || PlayerCommand::GetIsGrab() || PortalManager::GetCanMakePortal()) {
        sprite_->Draw();
    }

}

void RaySprite::SetTexture(const TextureUI num)
{
    sprite_->SetTextureHandle(handle_[num]);
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
        PC::IsRayHit()||
        Gentleman::IsRayHit()||
        Document::IsRayHit()||
        GiantGentleMan::IsRayHit();
}
