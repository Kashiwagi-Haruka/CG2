#include "WallManager2.h"
#include"Model/ModelManager.h"
#include"Function.h"
namespace {
    const int kMaxWall = 5;
}

WallManager2::WallManager2()
{
    room1_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/room2", "room2");
    room1_->SetModel("room2");
    roomMat_ = Function::MakeIdentity4x4();

    walls_.clear();

    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&roomMat_);
        walls_.push_back(std::move(wall));
    }

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

    plane_ = std::make_unique<Primitive>();
}


void WallManager2::Initialize()
{
    room1_->Initialize();
    plane_->Initialize(Primitive::Plane, "Resources/2d/white2x2.png");

    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }
}
void WallManager2::Update()
{
    plane_->SetEnableLighting(false);
    room1_->Update();
    //roomMat_ = room1_->GetWorldMatrix();
    Matrix4x4 planeMat = Function::MakeAffineMatrix(plane_->GetTransform().scale, plane_->GetTransform().rotate, plane_->GetTransform().translate);
    plane_->SetWorldMatrix(planeMat);
    plane_->Update();
    Vector3 normal = YoshidaMath::GetForward(plane_->GetWorldMatrix());
    normal.x *= 1.0f;
    areaLights_[1].normal = normal;
    areaLights_[1].position = YoshidaMath::GetWorldPosByMat(plane_->GetWorldMatrix()) - normal * 2.0f;
    areaLights_[1].height = plane_->GetTransform().scale.y*0.5f;
    Vector3 translate = {7.0f,0.0f,0.0f};

    walls_[0]->SetST({ 2.0f,4.0f,14.0f }, translate+Vector3{ 7.0f,2.0f,0.0f });
    walls_[1]->SetST({ 2.0f,4.0f,14.0f }, translate+Vector3{ -7.0f,2.0f,0.0f });
    //裏側                  
    walls_[2]->SetST({ 14.0f,4.0f,1.0f }, translate+Vector3{ 0.0f,2.0f,7.0f });
                                  
    walls_[3]->SetST({7.0f, 4.0f,1.0f,}, translate+Vector3{ -3.75f,2.0f,-6.5f });
    walls_[4]->SetST({7.0f, 4.0f,1.0f,}, translate+Vector3{ 3.75f,2.0f ,-6.5f });
   
    for (auto& wall : walls_) {
        wall->Update();
    }

}