#include "TimeCardWatch.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include<imgui.h>
#include"Camera.h"
#include"Audio.h"

bool TimeCardWatch::canMakePortal_ = false;

namespace {
    float tMin_ = 0.0f;
    float tMax_ = 5.0f;
}

TimeCardWatch::TimeCardWatch()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCard", "timeCard");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCard");

}

void TimeCardWatch::Initialize()
{
    canMakePortal_ = false;
    modelObj_->Initialize();
    transform_ = { .scale = {20.0f,20.0f,20.0f},.rotate = {-Function::kPi,0.0f,2.55f},.translate = {1.5f,-1.2f,1.5f} };
}

void TimeCardWatch::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    camera_ = camera;
    assert(camera_);
    modelObj_->UpdateCameraMatrices();
}

void TimeCardWatch::Update()
{
    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    assert(parentTransform_);
    Matrix4x4 parent = camera_->GetWorldMatrix();
    child = Function::Multiply(child, parent);

    modelObj_->SetWorldMatrix(child);
    modelObj_->Update();

#ifdef USE_IMGUI
    ImGui::Begin("timeCardWatch");
    ImGui::DragFloat3("translate", &transform_.translate.x, 0.3f);
    ImGui::DragFloat3("scale", &transform_.scale.x, 0.3f);
    ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.3f);
    ImGui::End();
#endif
}

void TimeCardWatch::Draw()
{
    modelObj_->Draw();   
}

bool TimeCardWatch::OnCollisionObjOfMakePortal(const Ray& ray, const AABB& aabb, const Transform& transform)
{
    //ポータル作れるよ
    canMakePortal_ = YoshidaMath::RayIntersectsAABB(ray, aabb, tMin_, tMax_);
    return canMakePortal_;
}

