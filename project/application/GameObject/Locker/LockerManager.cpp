#include "LockerManager.h"
#include <string>
#include"GameObject/KeyBindConfig.h"
#include"TextureManager.h"
#include"ScreenSize.h"
#include"SpriteCommon.h"
#include"GameObject/Player/Player.h"

bool LockerManager::isRayHit_ = false;
bool LockerManager::isInLocker_ = false;

LockerManager::LockerManager(const uint32_t num)
{
    for (int i = 0; i < num; ++i) {
        std::unique_ptr<Locker> locker = std::make_unique<Locker>();
        std::string name = "Locker" + std::to_string(i);
        locker->SetAnimationGroupName(name);
        locker->SetEditorRegistrationName("Locker" + std::to_string(i));
        lockers_.push_back(std::move(locker));
    }

    sprite_ = std::make_unique<Sprite>();
}

void LockerManager::Initialize()
{
    isRayHit_ = false;
    isInLocker_ = false;
    for (auto& locker : lockers_) {
        locker->Initialize();
    }

    uint32_t handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/locker.png");
    sprite_->Initialize(handle);
    sprite_->SetAnchorPoint({ 0.5f,0.5f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ SCREEN_SIZE::WIDTH,SCREEN_SIZE::HEIGHT });
    sprite_->SetPosition({ SCREEN_SIZE::HALF_WIDTH,SCREEN_SIZE::HALF_HEIGHT });
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();

}

void LockerManager::Update()
{
    for (auto& locker : lockers_) {
        locker->Update();
    }

    isRayHit_ = false;

    for (auto& locker : lockers_) {
        if (locker->IsRayHit())
        {
            isRayHit_ = true;
            break;
        }
    }

    sprite_->Update();
}

void LockerManager::InLocker(Player* player)
{


    for (auto& locker : lockers_) {

        if (locker->GetIsPlayerIn()) {

            if (locker->GetIsOpen()) {
                Vector3 forward = locker->GetForward();
                forward.x *= 0.5f;
                forward.z *= 0.5f;
                forward.y = 0.0f;
                Vector3 translate = player->GetTransform().translate;
                forward.x += translate.x;
                forward.y = translate.y;
                forward.z += translate.z;

                player->SetTranslate(forward);

                // ★ここでロッカー側の入室フラグも false に落とす
                locker->SetIsPlayerIn(false);
                isInLocker_ = false;
                break;
            } else {
                Vector3 pos = locker->GetWorldPosition();
                pos.y = player->GetTransform().translate.y;
                Vector3 rotate = locker->GetRotate();
                player->SetTranslate(pos);
                player->SetRotate(rotate);
                isInLocker_  = true;
                break;


            }
        }
    }



}

void LockerManager::Draw()
{
    for (auto& locker : lockers_) {
        locker->Draw();
    }



}

void LockerManager::SetCamera(Camera* camera)
{
    for (auto& locker : lockers_) {
        locker->SetCamera(camera);
    }
}

void LockerManager::SetPlayerCamera(PlayerCamera* playerCamera)
{
    Locker::SetPlayerCamera(playerCamera);
}

void LockerManager::DrawSprite()
{
    if (!isInLocker_) {
        return;
    }
    SpriteCommon::GetInstance()->DrawCommon();
    sprite_->Draw();

}
