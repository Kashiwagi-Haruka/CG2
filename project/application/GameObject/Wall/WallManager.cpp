#include "WallManager.h"

namespace {
    const int kMaxWall = 4;
}

WallManager::WallManager()
{
    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        walls_.push_back(std::move(wall));
    }
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
}

void WallManager::Update()
{
    for (auto& wall : walls_) {
        wall->Update();
    }
}

void WallManager::Draw()
{
    for (auto& wall : walls_) {
        wall->Draw();
    }
}

void WallManager::SetCamera(Camera* camera)
{
    for (auto& wall : walls_) {
        wall->SetCamera(camera);
    }
}
