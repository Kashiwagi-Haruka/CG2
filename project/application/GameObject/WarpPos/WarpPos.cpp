#include "WarpPos.h"
#include"Model/ModelManager.h"
#include<cmath>
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"

WarpPos::WarpPos()
{
    camera_ = std::make_unique<Camera>();
    object3d_ = std::make_unique<Object3d>();
    transform_= { .scale = {1.0f,1.0f,1.0f},.rotate = { 0.0f,Function::kPi,0.0f},.translate = { 0.0f,0.0f,0.0f}};
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/camera", "camera");
    object3d_->SetModel("camera");
    object3d_->SetTransform(transform_);

}
WarpPos::~WarpPos()
{
    object3d_.reset();
    camera_.reset();
}

Vector3 WarpPos::GetWorldPos()
{
    return YoshidaMath::GetWorldPosByMat(camera_->GetWorldMatrix());
}

void WarpPos::Initialize()
{
   
    object3d_->Initialize();
    sinTheta_ = 0.0f;
    camera_->SetFarClip(10000.0f);
    camera_->SetNearClip(0.001f);
    //camera_->SetFovY(-180.0f);
}

void WarpPos::SetCamera(Camera* camera)
{
    object3d_->SetCamera(camera);
    object3d_->UpdateCameraMatrices();
}

void WarpPos::Update()
{

    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    if (parentTransform_) {
        Matrix4x4 parent = Function::MakeAffineMatrix(parentTransform_->scale, parentTransform_->rotate, parentTransform_->translate);
        child = Function::Multiply( child, parent);
    } 

    object3d_->SetWorldMatrix(child);

    camera_->UpdateViewProjection(child);
    object3d_->Update();
}

void WarpPos::Draw()
{
 
}
