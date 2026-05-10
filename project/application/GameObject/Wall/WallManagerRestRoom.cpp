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
    //for (int i = 0; i < kMaxWall; ++i) {
    //    std::unique_ptr<Wall> wall = std::make_unique<Wall>();
    //    wall->SetParentMatrix(&roomMat_);
    //    walls_.push_back(std::move(wall));
    //}


    colliders_["FrontWall"] = std::make_unique<ObjectCollider>();
    colliders_["LeftWall"] = std::make_unique<ObjectCollider>();
    colliders_["RightWall"] = std::make_unique<ObjectCollider>();
    colliders_["Floor"] = std::make_unique<ObjectCollider>();
}

void WallManagerRestRoom::Update()
{
    room1_->Update();
    //for (auto& wall : walls_) {
    //    wall->Update();
    //}

    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }
}

void WallManagerRestRoom::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("restRoom");

    // 壁の初期化
    //for (auto& wall : walls_) {
    //    wall->Initialize();
    //}

    //std::vector<Primitive*> wallPrimitives;
    //wallPrimitives.reserve(walls_.size());
    //for (const auto& wall : walls_) {
    //    wallPrimitives.push_back(wall->GetPrimitive());
    //}


    //Vector3 translate = { 0.0f,0.0f,0.0f };

    //walls_[0]->SetST({ 2.0f,5.0f,2.0f }, translate);

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

    //for (auto& wall : walls_) {
    //    wall->Draw();
    //}

    //for (auto& [name, collider] : colliders_) {
    //    collider->Draw();
    //}
}

void WallManagerRestRoom::SetCamera(Camera* camera)
{

    room1_->SetCamera(camera);
    room1_->UpdateCameraMatrices();

    //for (auto& wall : walls_) {
    //    wall->SetCamera(camera);
    //}

    //for (auto& [name, hand] : colliders_) {
    //    hand->SetCamera(camera);
    //}
}
