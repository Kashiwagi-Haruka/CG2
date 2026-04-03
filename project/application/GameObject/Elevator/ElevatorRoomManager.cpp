#include "ElevatorRoomManager.h"
#include"Model/ModelManager.h"
#include"Function.h"

namespace {
    const int kMaxWall = 6;
}

ElevatorRoomManager::ElevatorRoomManager()
{
    room_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/elevatorRoom", "elevatorRoom");
    room_->SetModel("elevatorRoom");
    roomMat_ = Function::MakeIdentity4x4();

    walls_.clear();

    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&roomMat_);
        walls_.push_back(std::move(wall));
    }

    areaLight_.color = { 1.0f,1.0f,1.0f,1.0f };
    areaLight_.intensity = 1.0f;
    areaLight_.width = 10.0f;
    areaLight_.height = 4.0f;
    areaLight_.radius = 40.0f;
    areaLight_.decay = 0.1f;
    areaLight_.normal = {0.0f,-1.0f,0.0f};
}

ElevatorRoomManager::~ElevatorRoomManager()
{
    for (auto& wall : walls_) {
        if (wall != nullptr) {
            wall.reset();
            wall = nullptr;
        }
    }

    walls_.clear();
}

void ElevatorRoomManager::Initialize()
{
    room_->Initialize();

    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }
}

void ElevatorRoomManager::Update()
{
    room_->Update();
    roomMat_ = room_->GetWorldMatrix();

    areaLight_.position = YoshidaMath::GetWorldPosByMat(room_->GetWorldMatrix());

    walls_[0]->SetST({ 1.0f,4.0f,6.0f }, { -7.0f,2.0f,0.0f });
    walls_[1]->SetST({ 1.0f,4.0f,6.0f }, { 7.0f,2.0f,0.0f });

    walls_[2]->SetST({ 7.0f, 4.0f,1.0f, }, { -3.75f,2.0f,3.0f });
    walls_[3]->SetST({ 7.0f, 4.0f,1.0f, }, { 3.75f,2.0f ,3.0f });

    walls_[4]->SetST({ 7.0f, 4.0f,1.0f, },{ -3.75f,2.0f, -2.0f });
    walls_[5]->SetST({ 7.0f, 4.0f,1.0f },{ 3.75f,2.0f, -2.0f });

    for (auto& wall : walls_) {
        wall->Update();
    }

}

void ElevatorRoomManager::Draw()
{
    room_->Draw();

    //for (auto& wall : walls_) {
    //    wall->Draw();
    //}

}

void ElevatorRoomManager::SetCamera(Camera* camera)
{
    room_->SetCamera(camera);
    room_->UpdateCameraMatrices();

    for (auto& wall : walls_) { 
        wall->SetCamera(camera);
    }
}
