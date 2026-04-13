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

    walls_.clear();

    for (int i = 0; i < kMaxWall; ++i) {
        std::unique_ptr<Wall> wall = std::make_unique<Wall>();
        wall->SetParentMatrix(&roomMat_);
        walls_.push_back(std::move(wall));
    }

    areaLight_.color = { 1.0f,1.0f,1.0f,1.0f };
    areaLight_.intensity = 0.1f;
    areaLight_.width = 14.0f;
    areaLight_.height = 4.0f;
    areaLight_.radius = 10.0f;
    areaLight_.decay = 2.0f;

    plane_ = std::make_unique<Primitive>();

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
    plane_->Initialize(Primitive::Plane, "Resources/2d/white2x2.png");


    // 壁の初期化
    for (auto& wall : walls_) {
        wall->Initialize();
    }
}

void WallManager::Update()
{
    plane_->SetEnableLighting(false);
    room1_->Update();
    room1_->SetEnableLighting(true);
    roomMat_ = room1_->GetWorldMatrix();
    Matrix4x4 planeMat = Function::MakeAffineMatrix(plane_->GetTransform().scale, plane_->GetTransform().rotate, plane_->GetTransform().translate);

    plane_->SetWorldMatrix(planeMat);
    plane_->Update();
    Vector3 normal = YoshidaMath::GetForward(plane_->GetWorldMatrix());
    normal.x *= 1.0f;
    areaLight_.normal = normal;
    areaLight_.position = YoshidaMath::GetWorldPosByMat(plane_->GetWorldMatrix()) - normal * 2.0f;

    areaLight_.height = plane_->GetTransform().scale.y*0.5f;

    walls_[0]->SetST({ 2.0f,4.0f,14.0f }, { -7.0f ,0.0f,2.0f });
    walls_[1]->SetST({ 2.0f,4.0f,14.0f }, { 7.0f  ,0.0f,2.0f});

    walls_[2]->SetST({ 14.0f,4.0f,1.0f }, {2.0f,0.0f,-7.0f });
    walls_[3]->SetST({ 14.0f,4.0f,1.0f }, {2.0f,0.0f, 7.0f});

    for (auto& wall : walls_) {
        wall->Update();
    }

}

void WallManager::Draw()
{
    room1_->Draw();
    plane_->Draw();
    //for (auto& wall : walls_) {
    //    wall->Draw();
    //}

}

void WallManager::SetCamera(Camera* camera)
{

    room1_->SetCamera(camera);
    room1_->UpdateCameraMatrices();
    plane_->SetCamera(camera);
    plane_->UpdateCameraMatrices();

    //for (auto& wall : walls_) {
    //    wall->SetCamera(camera);
    //}
}
