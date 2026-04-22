#include "LockerManager.h"

bool LockerManager::isRayHit_ = false;

LockerManager::LockerManager()
{
    for (int i = 0; i < kMaxLockers_; ++i) {
        std::unique_ptr<Locker> locker = std::make_unique<Locker>();
        std::string name = "Locker" + std::to_string(i);
        locker->SetAnimationGroupName(name);
        lockers_.push_back(std::move(locker));
    }
}

void LockerManager::Initialize()
{
    isRayHit_ = false;
    for (auto& locker : lockers_) {
        locker->Initialize();
    }
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
