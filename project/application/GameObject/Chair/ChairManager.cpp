#include "ChairManager.h"
#include"Function.h"

namespace {
    uint32_t maxNum_ = 4;
}

bool ChairManager::isRayHit_ = false;
ChairManager::ChairManager()
{
    for (uint32_t i = 0; i < maxNum_; ++i) {
        std::unique_ptr<Chair> chair = std::make_unique<Chair>();
        chairs_.push_back(std::move(chair));
    }
}

ChairManager::~ChairManager()
{
    for (auto& chair : chairs_) {
        chair.reset();
    }
    chairs_.clear();
}

void ChairManager::SetCamera(Camera* camera)
{
    for (auto& chair:chairs_) {
        chair->SetCamera(camera);
    }


}

void ChairManager::Initialize()
{

    isRayHit_ = false;

    for (auto& chair : chairs_) {
        chair->Initialize();
    }
    std::vector<Object3d*> chairObjects;
    chairObjects.reserve(chairs_.size());
    for (const auto& chair : chairs_) {
        chairObjects.push_back(chair->GetObject3d());
    }
    Object3d::RegisterEditors(chairObjects, "Chair");



    for (size_t i = 0; i < maxNum_ - 1; i += 2) {
        Transform transform = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {6.25f+i*1.0f,1.0f,3.5f,}};
        chairs_.at(i)->SetTransform(transform);
        transform.rotate.y += Function::kPi;
        transform.translate.x *= -1.0f;
        chairs_.at(i+1)->SetTransform(transform);
        chairs_.at(i + 1)->SetMirrorTransform(&chairs_.at(i)->GetTransform());
        chairs_.at(i)->SetMirrorTransform(&chairs_.at(i + 1)->GetTransform());
    };
    
}

void ChairManager::Update()
{
    for (auto& chair : chairs_) {
        chair->Update();
    }
    isRayHit_ = false;

    for (auto& chair : chairs_) {
        if (chair->IsRayHit()) {
            isRayHit_ = true;
        }
    }
}

void ChairManager::Draw()
{
    for (auto& chair : chairs_) {
        chair->Draw();
    }
}

void ChairManager::SetPlayerCamera(PlayerCamera* camera)
{
    Chair::SetPlayerCamera(camera);
}
