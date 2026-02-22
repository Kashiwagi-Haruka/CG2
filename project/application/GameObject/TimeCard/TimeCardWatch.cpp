#include "TimeCardWatch.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include<imgui.h>

namespace {
    const constexpr float kRayDiff = 10.0f;
}

TimeCardWatch::TimeCardWatch()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCard", "timeCard");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCard");
    line_ = std::make_unique<Primitive>();
    ray_ = { .origin = {0.0f},.diff = {0.0f} };
}

void TimeCardWatch::Initialize()
{
    modelObj_->Initialize();
    line_->Initialize(Primitive::Line);
    transform_ = { .scale = {10.0f,10.0f,10.0f},.rotate = {-1.5f,0.0f,2.55f},.translate = {1.5f,-1.2f,0.75f} };
}

void TimeCardWatch::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    line_->SetCamera(camera);
}

void TimeCardWatch::Update()
{
    Vector3 end = {
    .x = ray_.origin.x + ray_.diff.x * kRayDiff,
    .y = ray_.origin.y + ray_.diff.y * kRayDiff,
    .z = ray_.origin.z + ray_.diff.z * kRayDiff,
    };

    line_->SetLinePositions(ray_.origin, end);

    //親子関係を付ける
    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    assert(parentTransform_);
    Matrix4x4 parent = Function::MakeAffineMatrix(parentTransform_->scale, parentTransform_->rotate, parentTransform_->translate);
    child = Function::Multiply(child, parent);

    modelObj_->SetWorldMatrix(child);
    modelObj_->Update();
    line_->Update();

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
    line_->Draw();
}

void TimeCardWatch::SetRay(const Vector3& origin, const Vector3& diff)
{
    ray_.origin = origin;
    ray_.diff = diff;


}
