#include "WarpPos1.h"
#include"Model/ModelManager.h"
#include<cmath>
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

WarpPos1::WarpPos1()
{
    camera_ = std::make_unique<Camera>();
    object3d_ = std::make_unique<Object3d>();
    transform_= { .scale = {1.0f,1.0f,1.0f},.rotate = { 0.0f,0.0f,0.0f},.translate = { 0.0f,1.0f,-10.0f}};
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/camera", "camera");
    object3d_->SetModel("camera");
    object3d_->SetTransform(transform_);
}

void WarpPos1::Initialize()
{
    object3d_->Initialize();
    sinTheta_ = 0.0f;
}

void WarpPos1::SetCamera(Camera* camera)
{
    object3d_->SetCamera(camera);
}

void WarpPos1::Update()
{
    //sinTheta_ += Function::kPi * YoshidaMath::kDeltaTime;
    //if (sinTheta_ >= Function::kPi*2.0f) {
    //    sinTheta_ = 0.0f;
    //}
    //transform_.translate.y += std::sinf(sinTheta_)*0.0625f;
    object3d_->SetTransform(transform_);
    object3d_->Update();
    

}

void WarpPos1::Draw()
{
    object3d_->Draw();
}
