#include "WallManagerRestRoom.h"
#include"Model/ModelManager.h"
#include"Function.h"

namespace {
    const int kMaxWall = 1;
}
WallManagerRestRoom::WallManagerRestRoom()
{

    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/restRoom", "restRoom");
    room1_->SetModel("restRoom");
    walls_.clear();

    //エリアライトの設定をしよう
    //areaLights_
    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&roomMat_);
        walls_.push_back(std::move(wall));
    }


    colliders_["FrontWall"] = std::make_unique<ObjectCollider>();
    colliders_["LeftWall"] = std::make_unique<ObjectCollider>();
    colliders_["RightWall"] = std::make_unique<ObjectCollider>();

}

void WallManagerRestRoom::Update()
{
    room1_->Update();
    for (auto& wall : walls_) {
        wall->Update();
    }

    for (auto& [name,collider] : colliders_) {
        collider->Update();
    }
}

void WallManagerRestRoom::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("restRoom");

    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }

    std::vector<Primitive*> wallPrimitives;
    wallPrimitives.reserve(walls_.size());
    for (const auto& wall : walls_) {
        wallPrimitives.push_back(wall->GetPrimitive());
    }

    const float offsetX = 0.0f;
    const float offsetZ = 0.0f;
    Vector3 translate = { 7.0f,0.0f,0.0f };

    //一つだけ床にしました。
    walls_[0]->SetST({ 14.0f,0.01f,30.0f }, translate + Vector3{ 0.0f + offsetX,0.0f,-7.0f });
    walls_[0]->SetCollisionAttribute(kCollisionFloor);
    walls_[0]->SetCollisionMask(kCollisionPlayer | kCollisionChair | kCollisionKey | kCollisionItem);


    colliders_["FrontWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["LeftWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["RightWall"]->Initialize(YoshidaMath::ColliderType::kAABB);

    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }



}

void WallManagerRestRoom::Draw()
{
    room1_->Draw();

    for (auto& wall : walls_) {
        wall->Draw();
    }

    for (auto& [name, collider] : colliders_) {
        collider->Draw();
    }
}

void WallManagerRestRoom::SetCamera(Camera* camera)
{

    room1_->SetCamera(camera);
    room1_->UpdateCameraMatrices();

    for (auto& wall : walls_) {
        wall->SetCamera(camera);
    }

    for (auto& [name, hand] : colliders_) {
        hand->SetCamera(camera);
    }
}
