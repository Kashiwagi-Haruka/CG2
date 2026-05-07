#include "WallManagerRoofFoor.h"
#include"Model/ModelManager.h"
#include"Function.h"

namespace {
    const int kMaxWall = 3;
}
WallManagerRoofFoor::WallManagerRoofFoor()
{

    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/roofTop", "roofTop");
    room1_->SetModel("roofTop");
    walls_.clear();

    //エリアライトの設定をしよう
    //areaLights_
    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&roomMat_);
        walls_.push_back(std::move(wall));
    }
}

void WallManagerRoofFoor::Update()
{

    for (auto& wall : walls_) {
        wall->Update();
    }
}

void WallManagerRoofFoor::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("roofTop");


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
    walls_[0]->SetST({ 2.0f,4.0f,14.0f }, translate + Vector3{ 7.0f + offsetX,0.0f,0.0f + offsetZ });
    walls_[1]->SetST({ 2.0f,4.0f,14.0f }, translate + Vector3{ -7.0f + offsetX,0.0f,0.0f + offsetZ });
    walls_[2]->SetST({ 14.0f,4.0f,1.0f }, translate + Vector3{ 0.0f + offsetX,0.0f,7.0f + offsetZ });
}
