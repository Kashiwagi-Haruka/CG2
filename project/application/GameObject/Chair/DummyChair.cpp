#include "DummyChair.h"
#include "Object3d/Object3dCommon.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"

DummyChair::DummyChair()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/chair", "chair");
    obj_->SetModel("chair");

    SetAABB({
   .min = {-0.125f, 0.0f, -0.125f},
     .max = {0.125f,  0.5f, 0.125f }
        });
}

void DummyChair::OnCollision(Collider* collider)
{
}

Vector3 DummyChair::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void DummyChair::Update()
{
    obj_->Update();
}

void DummyChair::Initialize()
{
    obj_->Initialize();
    //紳士用の椅子
    obj_->RegisterEditor("DummyChair");
}

void DummyChair::Draw()
{
    obj_->Draw();
}

void DummyChair::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}
