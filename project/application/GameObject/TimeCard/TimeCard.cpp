#include "TimeCard.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include<imgui.h>
#include"Camera.h"
#include"Audio.h"

TimeCard::TimeCard()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCard", "timeCard");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCard");
}

void TimeCard::Initialize()
{
    modelObj_->Initialize();
    modelObj_->RegisterEditor("TimeCard");
}

void TimeCard::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    modelObj_->UpdateCameraMatrices();
}

void TimeCard::Update()
{
    modelObj_->Update();
}

void TimeCard::Draw()
{
    modelObj_->Draw();
}

