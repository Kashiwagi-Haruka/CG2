#include "DeskManager.h"

DeskManager::DeskManager()
{
    for (int i = 0; i < kMaxDesks_; ++i) {
        std::unique_ptr<Desk> desk = std::make_unique<Desk>();
        std::string name = "Desk" + std::to_string(i);
        desk->SetAnimationGroupName(name);
        desks_.push_back(std::move(desk));
    }
}

void DeskManager::Initialize()
{
    for (auto& desk : desks_) {
        desk->Initialize();
    }
}

void DeskManager::Update()
{
    for (auto& desk : desks_) {
        desk->Update();
    }
}

void DeskManager::Draw()
{
    for (auto& desk : desks_) {
        desk->Draw();
    }
}

void DeskManager::SetCamera(Camera* camera)
{
    for (auto& desk : desks_) {
        desk->SetCamera(camera);
    }
}

void DeskManager::SetPlayerCamera(PlayerCamera* playerCamera)
{
    Desk::SetPlayerCamera(playerCamera);
}
