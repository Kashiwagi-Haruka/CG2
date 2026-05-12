#include "WallManagerRestRoom.h"
#include"Model/ModelManager.h"
#include"Function.h"

WallManagerRestRoom::WallManagerRestRoom()
{

    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/restRoom", "restRoom");
    room1_->SetModel("restRoom");

}

void WallManagerRestRoom::Update()
{
    room1_->Update();

    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }
}

void WallManagerRestRoom::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("restRoom");

    colliders_.clear();

    colliders_["FrontWall"] = std::make_unique<ObjectCollider>();
    colliders_["LeftWall"] = std::make_unique<ObjectCollider>();
    colliders_["RightWall"] = std::make_unique<ObjectCollider>();
    colliders_["Floor"] = std::make_unique<ObjectCollider>();

    colliders_["FrontWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["LeftWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["RightWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["Floor"]->Initialize(YoshidaMath::ColliderType::kAABB);

    colliders_["Floor"]->SetCollisionAttribute(kCollisionFloor);
    colliders_["Floor"]->SetCollisionMask(kCollisionPlayer | kCollisionChair | kCollisionKey | kCollisionItem);

    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }
}

void WallManagerRestRoom::Draw()
{
    room1_->Draw();

    //for (auto& [name, collider] : colliders_) {
    //    collider->Draw();
    //}
}

void WallManagerRestRoom::SetCamera(Camera* camera)
{

    room1_->SetCamera(camera);
    room1_->UpdateCameraMatrices();

    //for (auto& [name, hand] : colliders_) {
    //    hand->SetCamera(camera);
    //}
}
