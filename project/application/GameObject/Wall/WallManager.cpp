#include "WallManager.h"
#include"Model/ModelManager.h"
#include"Function.h"

namespace {
    const int kMaxWall = 4;
}

WallManager::WallManager()
{
    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/room1", "room1");
    room1_->SetModel("room1");
    roomMat_ = Function::MakeIdentity4x4();
    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&roomMat_);
        walls_.push_back(std::move(wall));
    }

}

WallManager::~WallManager()
{
    for (auto& wall : walls_) {
        if (wall != nullptr) {
            wall.reset();
            wall = nullptr;
        }
    }

    walls_.clear();
}

void WallManager::Initialize()
{
    room1_->Initialize();
    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }
}

void WallManager::Update()
{
    room1_->Update();
    roomMat_ = room1_->GetWorldMatrix();

    walls_[0]->SetST({ 1.0f,4.0f,15.0f }, { 0.0f,2.0f,-7.5f });
    walls_[1]->SetST({ 1.0f,4.0f,15.0f }, { 0.0f,2.0f,7.5f });

    walls_[2]->SetST({ 15.0f,4.0f,1.0f }, { 7.5f,2.0f,0.0f });
    walls_[3]->SetST({ 15.0f,4.0f,1.0f }, { -7.5f,2.0f,0.0f });

    for (auto& wall : walls_) {
        wall->Update();
    }

}

void WallManager::Draw()
{
    room1_->Draw();

    //for (auto& wall : walls_) {
    //    wall->Draw();
    //}

}

void WallManager::SetCamera(Camera* camera)
{
   
    room1_->SetCamera(camera);
    room1_->UpdateCameraMatrices();
    for (auto& wall : walls_) {
        wall->SetCamera(camera);
    }
}
