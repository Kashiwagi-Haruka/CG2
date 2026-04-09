#include "TimeCardWatch.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include<imgui.h>
#include"Camera.h"
#include"Audio.h"
#include"GameObject/Player/Player.h"

TimeCardWatch::TimeCardWatch()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCardWatch", "timeCardWatch");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCardWatch");
}

void TimeCardWatch::Initialize()
{
    modelObj_->Initialize();
    transform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.04f,0.0f} };
}

void TimeCardWatch::SetCamera(Camera* camera)
{
    camera_ = camera;
    assert(camera_);
    //カメラのセット
    modelObj_->SetCamera(camera);
    modelObj_->UpdateCameraMatrices();
}

void TimeCardWatch::Update()
{
    Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    assert(player_);
    Matrix4x4 parent =player_->GetJointMatrix("Hand.R");
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

