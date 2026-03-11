#include "WallManager.h"
#include"Model/ModelManager.h"

namespace {
    const int kMaxWall = 4;
}

WallManager::WallManager()
{
    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        walls_.push_back(std::move(wall));
    }

    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/room1", "room1");
    room1_->SetModel("room1");
}

WallManager::~WallManager()
{
    for (auto& wall:walls_) {
        if (wall != nullptr) {
            wall.reset();
            wall = nullptr;
        }
    }

    walls_.clear();
}

void WallManager::Initialize()
{
    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }

    room1_->Initialize();
}

void WallManager::Update()
{
    for (auto& wall : walls_) {
        wall->Update();
    }

    room1_->Update();
}

void WallManager::Draw()
{
 /*   for (auto& wall : walls_) {
        wall->Draw();
    }*/
    room1_->Draw();
}

void WallManager::SetCamera(Camera* camera)
{
    for (auto& wall : walls_) {
        wall->SetCamera(camera);
    }

    room1_->SetCamera(camera);
}
