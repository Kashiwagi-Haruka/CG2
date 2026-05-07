#include "TimeCardRack.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include<imgui.h>
#include"Camera.h"
#include"Audio.h"

TimeCardRack::TimeCardRack()
{
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/timeCardRack", "timeCardRack");
    modelObj_ = std::make_unique<Object3d>();
    modelObj_->SetModel("timeCardRack");
}

void TimeCardRack::Initialize()
{
    modelObj_->Initialize();
    modelObj_->RegisterEditor("TimeCardRack");
}

void TimeCardRack::SetCamera(Camera* camera)
{
    //カメラのセット
    modelObj_->SetCamera(camera);
    modelObj_->UpdateCameraMatrices();
}

void TimeCardRack::Update()
{
    modelObj_->Update();
}

void TimeCardRack::Draw()
{
    modelObj_->Draw();
}

