#include "DeskManager.h"
#include "Mesh/Object3d/Object3dCommon.h"
bool DeskManager::isRayHit_ = false;

DeskManager::DeskManager(const uint32_t deskCount)
{
    for (uint32_t i = 0; i < deskCount; ++i) {
        std::unique_ptr<Desk> desk = std::make_unique<Desk>();
        std::string name = "Desk" + std::to_string(i);
        desk->SetAnimationGroupName(name);
        desks_.push_back(std::move(desk));
    }
}

void DeskManager::Initialize()
{

    isRayHit_ = false;

    for (auto& desk : desks_) {
        desk->Initialize();
    }
}

void DeskManager::Update()
{
    for (auto& desk : desks_) {
        desk->Update();
    }

    isRayHit_ = false;

    for (auto& desk : desks_) {
        if (desk->IsRayHit()) {
            isRayHit_ = true;
            break;
        };
    }
}

void DeskManager::Draw()
{
    Object3dCommon::GetInstance()->DrawCommonSkinning();
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

Matrix4x4* DeskManager::GetDrawerMatrix(const uint32_t deskNum)
{
    assert(deskNum < desks_.size());
    return desks_[deskNum]->GetDeskDrawerMatrix();
}
