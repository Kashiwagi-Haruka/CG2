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

    areaLight_.color = { 1.0f,1.0f,1.0f,1.0f };
    areaLight_.intensity = 1.0f;
    areaLight_.width = 14.0f;
    areaLight_.height = 4.0f;
    areaLight_.radius = 7.0f;
    areaLight_.decay = 0.1f;

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
    Function::Multiply(planeMat, roomMat_);
    plane_->SetWorldMatrix(planeMat);
    plane_->Update();
    Vector3 normal = YoshidaMath::GetForward(plane_->GetWorldMatrix());
    normal.x *= 1.0f;
    areaLight_.normal = normal;
    areaLight_.position = YoshidaMath::GetWorldPosByMat(plane_->GetWorldMatrix()) - normal * 2.0f;
    areaLight_.height = plane_->GetTransform().scale.y*0.5f;
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