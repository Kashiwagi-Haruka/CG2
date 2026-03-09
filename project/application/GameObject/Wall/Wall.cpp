#include "Wall.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

Wall::Wall()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/chair", "chair");
    obj_->SetModel("chair");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.25f,0.5f,0.25f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);
}

void Wall::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        obj_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
    } else {
        obj_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    }
}

Vector3 Wall::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void Wall::Update()
{
    obj_->SetTransform(transform_);
    obj_->Update();
}

void Wall::Initialize()
{
    obj_->Initialize();
    transform_.translate = { 1.0f,0.1f,1.0f };
    transform_.rotate = { 0.0f,0.0f,0.0f };
    transform_.scale = { 1.0f,1.0f,1.0f };
}

void Wall::Draw()
{
    obj_->UpdateCameraMatrices();
    obj_->Draw();
}


void Wall::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
}
