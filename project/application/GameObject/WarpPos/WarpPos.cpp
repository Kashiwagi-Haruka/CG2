#include "WarpPos.h"
#include"Model/ModelManager.h"
#include<cmath>
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

WarpPos::WarpPos()
{
    camera_ = std::make_unique<Camera>();
    object3d_ = std::make_unique<Object3d>();
    transform_= { .scale = {0.1f,0.1f,0.1f},.rotate = { 0.0f,0.0f,0.0f},.translate = { 0.0f,0.0f,0.0f}};
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/camera", "camera");
    object3d_->SetModel("camera");
    object3d_->SetTransform(transform_);
}

void WarpPos::Initialize()
{
   
    object3d_->Initialize();
    sinTheta_ = 0.0f;
}

void WarpPos::SetCamera(Camera* camera)
{
    object3d_->SetCamera(camera);
}

void WarpPos::Update()
{
    sinTheta_ += Function::kPi * YoshidaMath::kDeltaTime;
   
    if (sinTheta_ >= Function::kPi*2.0f) {
        sinTheta_ = 0.0f;
    }

    transform_.translate.y += std::sinf(sinTheta_)*0.0625f;

    camera_->SetTransform(transform_);
    camera_->Update();
    object3d_->SetTransform(transform_);
    object3d_->Update();
}

void WarpPos::Draw()
{
    object3d_->UpdateCameraMatrices();
    object3d_->Draw();
}
