#include "Flashlight.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"

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
   
    spotLight_.position = obj_->GetTranslate();
    spotLight_.direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 10.0f;
   
}

void Flashlight::Initialize()
{
    obj_->Initialize();
    SetLight();
}

void Flashlight::Draw()
{
    obj_->UpdateCameraMatrices();
    obj_->Draw();
}

void Flashlight::SetLight()
{
    spotLight_.color = { 1.0f, 1.0f, 0.5f, 1.0f };
    spotLight_.position = obj_->GetTranslate();
    spotLight_.direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 10.0f;
    spotLight_.distance = 7.0f;
    spotLight_.decay = 2.0f;
    spotLight_.cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLight_.cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
}
