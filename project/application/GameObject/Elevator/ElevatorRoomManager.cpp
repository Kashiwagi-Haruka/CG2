#include "ElevatorRoomManager.h"
#include"Model/ModelManager.h"
#include"Function.h"

//namespace {
//    const int kMaxWall = 6;
//}

ElevatorRoomManager::ElevatorRoomManager()
{
    room_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/elevatorRoom", "elevatorRoom");
    room_->SetModel("elevatorRoom");
    roomMat_ = Function::MakeIdentity4x4();


    areaLight_.color = { 1.0f,1.0f,0.75f,1.0f };
    areaLight_.intensity = 1.0f;
    areaLight_.width = 2.0f;
    areaLight_.height = 2.0f;
    areaLight_.radius = 8.0f;
    areaLight_.decay = 2.0f;
    areaLight_.normal = { 0.0f,1.0f,0.0f };
}

ElevatorRoomManager::~ElevatorRoomManager()
{
}

void ElevatorRoomManager::Initialize()
{
    room_->Initialize();
    room_->RegisterEditor("room");

    colliders_.clear();

    colliders_["EV_LeftWall"] = std::make_unique<ObjectCollider>();
    colliders_["EV_RightWall"] = std::make_unique<ObjectCollider>();
    colliders_["EV_FrontWall_L"] = std::make_unique<ObjectCollider>();
    colliders_["EV_FrontWall_R"] = std::make_unique<ObjectCollider>();
    colliders_["EV_BackWall_R"] = std::make_unique<ObjectCollider>();
    colliders_["EV_BackWall_R"] = std::make_unique<ObjectCollider>();

    colliders_["EV_LeftWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_RightWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_FrontWall_L"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_FrontWall_R"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_BackWall_R"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["EV_BackWall_R"]->Initialize(YoshidaMath::ColliderType::kAABB);

    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }
}

void ElevatorRoomManager::Update()
{
    room_->Update();
    roomMat_ = room_->GetWorldMatrix();
    Vector3 translate = YoshidaMath::GetWorldPosByMat(room_->GetWorldMatrix());
    //translate.y += 4.0f;
    areaLight_.position = translate;

    //walls_[0]->SetST({ 1.0f,4.0f,6.0f }, { -7.0f,2.0f,0.0f });
    //walls_[1]->SetST({ 1.0f,4.0f,6.0f }, { 7.0f,2.0f,0.0f });

    ////部屋と部屋の通路の壁
    //walls_[2]->SetST({ 2.5f, 4.0f,1.0f, }, { -6.0f ,2.0f,3.0f });
    //walls_[3]->SetST({ 11.5f, 4.0f,1.0f, }, { 1.5f ,2.0f,3.0f  });

    //walls_[4]->SetST({ 7.0f, 4.0f,1.0f, },{ -4.25f,2.0f, -2.0f });
    //walls_[5]->SetST({ 7.0f, 4.0f,1.0f },{ 4.25f,2.0f, -2.0f });


    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }

}

void ElevatorRoomManager::Draw()
{
    room_->Draw();

    //for (auto& [name, collider] : colliders_) {
    //    collider->Draw();
    //}
}

void ElevatorRoomManager::SetCamera(Camera* camera)
{
    room_->SetCamera(camera);
    room_->UpdateCameraMatrices();

    //for (auto& [name, collider] : colliders_) {
    //    collider->SetCamera(camera);
    //}
}
