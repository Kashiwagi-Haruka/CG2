#include "WallManager.h"
#include"Model/ModelManager.h"
#include"Function.h"

WallManager::WallManager()
{
    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/room1", "room1");
    room1_->SetModel("room1");
    roomMat_ = Function::MakeIdentity4x4();





    //天井の照明
    areaLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[0].position = { 7.0f, 3.0f, 0.0f };
    areaLights_[0].normal = { 0.0f, 1.0f, 0.0f };
    areaLights_[0].intensity = 10.0f;
    areaLights_[0].width = 4.0f;
    areaLights_[0].height = 0.1f;
    areaLights_[0].radius = 4.0f;
    areaLights_[0].decay = 2.0f;

    //窓の光
    areaLights_[1].color = { 1.0f,1.0f,1.0f,1.0f };
    areaLights_[1].intensity = 0.1f;
    areaLights_[1].width = 14.0f;
    areaLights_[1].height = 4.0f;
    areaLights_[1].radius = 10.0f;
    areaLights_[1].decay = 2.0f;

    /*plane_ = std::make_unique<Primitive>();*/

}

WallManager::~WallManager()
{

}

void WallManager::Initialize()
{
    room1_->Initialize();
    room1_->RegisterEditor("room1");

    colliders_.clear();

    colliders_["FrontWall"] = std::make_unique<ObjectCollider>();
    colliders_["LeftWall"] = std::make_unique<ObjectCollider>();
    colliders_["RightWall"] = std::make_unique<ObjectCollider>();
    colliders_["BackWall"] = std::make_unique<ObjectCollider>();

    colliders_["FrontWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["LeftWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["RightWall"]->Initialize(YoshidaMath::ColliderType::kAABB);
    colliders_["BackWall"]->Initialize(YoshidaMath::ColliderType::kAABB);


    for (auto& [name, collider] : colliders_) {
        collider->RegisterEditor(name);
    }

    //walls_[0]->SetST({ 2.0f,4.0f,14.0f }, { -7.0f  ,0.0f,0.0f });
    //walls_[1]->SetST({ 2.0f,4.0f,14.0f }, { 7.0f   ,0.0f,0.0f });

    //walls_[2]->SetST({ 14.0f,4.0f,1.0f }, { 0.0f ,0.0f,-7.0f });
    //walls_[3]->SetST({ 14.0f,4.0f,1.0f }, { 0.0f ,0.0f, 7.0f });
    


}

void WallManager::Update()
{
    room1_->Update();
    room1_->SetEnableLighting(true);
    roomMat_ = room1_->GetWorldMatrix();

    for (auto& [name, collider] : colliders_) {
        collider->Update();
    }

}

void WallManager::Draw()
{
    room1_->Draw();

    for (auto& [name, collider] : colliders_) {
        collider->Draw();
    }
}

void WallManager::SetCamera(Camera* camera)
{
    room1_->SetCamera(camera);
    room1_->UpdateCameraMatrices();

    for (auto& [name, collider] : colliders_) {
        collider->SetCamera(camera);
    }
}
