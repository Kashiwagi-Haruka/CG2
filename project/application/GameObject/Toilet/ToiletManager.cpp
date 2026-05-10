#include "ToiletManager.h"
#include <string>

bool ToiletManager::isRayHit_ = false;

ToiletManager::ToiletManager()
{
    for (int i = 0; i < kMaxToilets_; ++i) {
        std::unique_ptr<Toilet> toilet = std::make_unique<Toilet>();
        std::string name = "Toilet" + std::to_string(i);
        toilet->SetAnimationGroupName(name);
        toilet->SetEditorRegistrationName("Toilet" + std::to_string(i));
        toilets_.push_back(std::move(toilet));
    }
}

void ToiletManager::Initialize()
{
    isRayHit_ = false;
    for (auto& toilet : toilets_) {
        toilet->Initialize();
    }
}

void ToiletManager::Update()
{
    for (auto& toilet : toilets_) {
        toilet->Update();
    }

    isRayHit_ = false;

    for (auto& toilet : toilets_) {
        if (toilet->IsRayHit())
        {
            isRayHit_ = true;
            break;
        }
    }
}

void ToiletManager::Draw()
{
    for (auto& toilet : toilets_) {
        toilet->Draw();
    }
}

void ToiletManager::SetCamera(Camera* camera)
{
    for (auto& toilet : toilets_) {
        toilet->SetCamera(camera);
    }
}

void ToiletManager::SetPlayerCamera(PlayerCamera* playerCamera)
{
    Toilet::SetPlayerCamera(playerCamera);
}