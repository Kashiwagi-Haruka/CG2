#include "Flashlight.h"
#include"Model/ModelManager.h"

Flashlight::Flashlight()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/light", "light");
    obj_->SetModel("light");
    SetAABB({ .min = {-0.1f,-0.2f,-0.1f},.max = {0.1f,0.1f,0.1f} });
    
}

void Flashlight::OnCollision(Collider* collider)
{

}

Vector3 Flashlight::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void Flashlight::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
}

void Flashlight::Update()
{
    obj_->Update();
}

void Flashlight::Initialize()
{
    obj_->Initialize();
    obj_->SetTranslate({0.0f,0.0f,0.0f});
}

void Flashlight::Draw()
{
    obj_->UpdateCameraMatrices();
    obj_->Draw();
}
