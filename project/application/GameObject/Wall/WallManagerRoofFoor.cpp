#include "WallManagerRoofFoor.h"
#include"Model/ModelManager.h"
#include"Function.h"


WallManagerRoofFoor::WallManagerRoofFoor()
{

    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/roofTop", "roofTop");
    room1_->SetModel("roofTop");

}

void WallManagerRoofFoor::Update()
{
    room1_->Update();

    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }
}

void WallManagerRoofFoor::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("roofTop");

    colliders_.clear();

    colliders_["FrontWall"] = std::make_unique<ObjectCollider>();
    colliders_["LeftWall"] = std::make_unique<ObjectCollider>();
    colliders_["RightWall"] = std::make_unique<ObjectCollider>();
    colliders_["Floor"] = std::make_unique<ObjectCollider>();

    colliders_["FrontWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["LeftWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["RightWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    //一つだけ床にしました。
    colliders_["Floor"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["Floor"]->SetCollisionAttribute(kCollisionFloor);
    colliders_["Floor"]->SetCollisionMask(kCollisionPlayer | kCollisionChair | kCollisionKey | kCollisionItem);

    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }

}
