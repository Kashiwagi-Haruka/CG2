#include "Portal.h"
#include"Function.h"
#include"application/GameObject/YoshidaMath/YoshidaMath.h"
Portal::Portal()
{
    ring_ = std::make_unique<Primitive>();
}

void Portal::Initialize()
{
    transform_ = { .scale = {1.5f,3.0f,1.5f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,1.5f,0.0f} };
    ring_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/atHome.jpg");
    ring_->SetTransform(transform_);
    ring_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    uvTranslateY_ = 0.0f;
    uvMat_ = Function::MakeTranslateMatrix({ 0.0f,uvTranslateY_ ,0.0f });
    sphere_ = { .center = {transform_.translate},.radius = 2.5f };
}

void Portal::Update()
{
    ring_->Update();
    uvTranslateY_ += YoshidaMath::kDeltaTime;
   uvMat_ =  Function::MakeTranslateMatrix({ 0.0f,uvTranslateY_ ,0.0f });
    ring_->SetUvTransform(uvMat_);
;}

void Portal::Draw()
{
    ring_->Draw();
}

void Portal::SetCamera(Camera* camera)
{
    ring_->SetCamera(camera);
}

const Sphere& Portal::GetSphere()
{
    sphere_ = { .center = {transform_.translate},.radius = 0.5f };
    return sphere_;
}
