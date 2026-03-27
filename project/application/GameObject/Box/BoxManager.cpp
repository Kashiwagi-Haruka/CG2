#include "BoxManager.h"
#include"Function.h"

namespace {
    uint32_t maxNum_ = 2;
}

BoxManager::BoxManager()
{
    for (uint32_t i = 0; i < maxNum_; ++i) {
        std::unique_ptr<Box> box = std::make_unique<Box>();
        boxes_.push_back(std::move(box));
    }
}

BoxManager::~BoxManager()
{
    for (auto& box : boxes_) {
        box.reset();
    }
    boxes_.clear();
}

void BoxManager::SetCamera(Camera* camera)
{
    for (auto& box : boxes_) {
        box->SetCamera(camera);
    }
}

void BoxManager::Initialize()
{
    for (auto& box : boxes_) {
        box->Initialize();
    }

    for (size_t i = 0; i < maxNum_ - 1; i += 2) {
        Transform transform = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {6.0f + i * 1.0f,1.5f,0.0f,} };
        boxes_.at(i)->SetTransform(transform);
        transform.rotate.y += Function::kPi;
        transform.translate.x *= -1.0f;
        boxes_.at(i + 1)->SetTransform(transform);
        boxes_.at(i + 1)->SetMirrorTransform(&boxes_.at(i)->GetTransform());
        boxes_.at(i)->SetMirrorTransform(&boxes_.at(i + 1)->GetTransform());
    };

}

void BoxManager::Update()
{
    for (auto& box : boxes_) {
        box->Update();
    }
}

void BoxManager::Draw()
{
    for (auto& box : boxes_) {
        box->Draw();
    }
}

void BoxManager::SetPlayerCamera(PlayerCamera* camera)
{
    Box::SetPlayerCamera(camera);
}
