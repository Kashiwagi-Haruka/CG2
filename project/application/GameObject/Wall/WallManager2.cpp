#include "WallManager2.h"
#include"Model/ModelManager.h"
#include"Function.h"

WallManager2::WallManager2()
{
    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/room2", "room2");
    room1_->SetModel("room2");
    roomMat_ = Function::MakeIdentity4x4();

    //天井
    areaLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[0].position = { 7.0f, 3.0f, 0.0f };
    areaLights_[0].normal = { 0.0f, 1.0f, 0.0f };
    areaLights_[0].intensity = 10.0f;
    areaLights_[0].width = 4.0f;
    areaLights_[0].height = 0.1f;
    areaLights_[0].radius = 4.0f;
    areaLights_[0].decay = 2.0f;

    //窓
    areaLights_[1].color = { 1.0f,1.0f,1.0f,1.0f };
    areaLights_[1].intensity = 0.1f;
    areaLights_[1].width = 14.0f;
    areaLights_[1].height = 4.0f;
    areaLights_[1].radius = 10.0f;
    areaLights_[1].decay = 2.0f;

}


void WallManager2::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("room2");

    colliders_.clear();
    colliders_["LeftWall2"] = std::make_unique<ObjectCollider>();
    colliders_["RightWall2"] = std::make_unique<ObjectCollider>();
    colliders_["BackWall2"] = std::make_unique<ObjectCollider>();

    colliders_["LeftWall2"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["RightWall2"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["BackWall2"]->Initialize(YoshidaMath::ColliderType::kAABB);

    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }

    //Vector3 translate = { 7.0f,0.0f,0.0f };
    //walls_[0]->SetST({ 2.0f,4.0f,14.0f }, translate + Vector3{ 7.0f ,0.0f,0.0f });
    //walls_[1]->SetST({ 2.0f,4.0f,14.0f }, translate + Vector3{ -7.0f ,0.0f,0.0f });
    //walls_[2]->SetST({ 14.0f,4.0f,1.0f }, translate + Vector3{ 0.0f ,0.0f,7.0f });
    //walls_[3]->SetST({ 2.5f, 4.0f,1.0f, }, translate + Vector3{ -6.0f ,0.0f,-6.5f });
    //walls_[4]->SetST({ 11.5f, 4.0f,1.0f, }, translate + Vector3{ 1.5f ,0.0f ,-6.5f });

}
void WallManager2::Update()
{
    room1_->Update();

    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }

}
