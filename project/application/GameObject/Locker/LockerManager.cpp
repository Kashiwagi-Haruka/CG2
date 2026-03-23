#include "LockerManager.h"

LockerManager::LockerManager()
{
    for (int i = 0; i < kMaxLockers_; ++i) {
        std::unique_ptr<Locker> locker = std::make_unique<Locker>();
        lockers_.push_back(std::move(locker));
    }
}

void LockerManager::Initialize()
{
    for (auto& locker : lockers_) {
        locker->Initialize();
    }
}

void LockerManager::Update()
{
    for (auto& locker : lockers_) {
        locker->Update();
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
