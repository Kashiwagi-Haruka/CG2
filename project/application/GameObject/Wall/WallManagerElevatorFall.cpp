#include "WallManagerElevatorFall.h"
#include"Model/ModelManager.h"
#include"Function.h"
WallManagerElevatorFall::WallManagerElevatorFall()
{
    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/elevatorHole", "elevatorHole");
    room1_->SetModel("elevatorHole");
    roomMat_ = Function::MakeIdentity4x4();

    aabb_ = {
     .min = {-6.3f, -28.0f, -5.1f},
     .max = { 6.3f,  28.0f,  5.1f}
    };

    ////天井
    //areaLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    //areaLights_[0].position = { 7.0f, 3.0f, 0.0f };
    //areaLights_[0].normal = { 0.0f, 1.0f, 0.0f };
    //areaLights_[0].intensity = 10.0f;
    //areaLights_[0].width = 4.0f;
    //areaLights_[0].height = 0.1f;
    //areaLights_[0].radius = 4.0f;
    //areaLights_[0].decay = 2.0f;

    ////窓
    //areaLights_[1].color = { 1.0f,1.0f,1.0f,1.0f };
    //areaLights_[1].intensity = 0.1f;
    //areaLights_[1].width = 14.0f;
    //areaLights_[1].height = 4.0f;
    //areaLights_[1].radius = 10.0f;
    //areaLights_[1].decay = 2.0f;

}


void WallManagerElevatorFall::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("elevatorHole");

    colliders_.clear();

    colliders_["EV_Hole_L"] = std::make_unique<ObjectCollider>();
    colliders_["EV_Hole_R"] = std::make_unique<ObjectCollider>();
    colliders_["EV_Hole_B"] = std::make_unique<ObjectCollider>();
    colliders_["EV_Hole_F"] = std::make_unique<ObjectCollider>();

    colliders_["EV_Hole_L"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_Hole_R"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_Hole_B"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_Hole_F"]->Initialize(YoshidaMath::ColliderType::kAABB);

    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }

}
void WallManagerElevatorFall::Update()
{
    room1_->Update();

    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }

}
