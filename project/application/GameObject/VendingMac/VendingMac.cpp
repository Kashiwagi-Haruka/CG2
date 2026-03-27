#include "VendingMac.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include<imgui.h>
#include"GameObject/SEManager/SEManager.h"

VendingMac::VendingMac()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/vendingMac", "vendingMac");
    obj_->SetModel("vendingMac");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,1.83f,0.5f} });
    SetCollisionAttribute(kCollisionVendingMac);
    SetCollisionMask(kCollisionPlayer);

    areaLight_.color = { 1.0f,234.0f / 255.0f,200.0f / 255.0f,1.0f };
    areaLight_.intensity = 3.1f;
    areaLight_.width = 2.0f;
    areaLight_.height = 0.8f;
    areaLight_.radius = 0.8f;
    areaLight_.decay = 1.0f;
    translate_ = { 0.0f,1.3f,0.6f };




}

VendingMac::~VendingMac()
{

}

void VendingMac::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 VendingMac::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void VendingMac::Update()
{
    obj_->Update();

    Matrix4x4 worldMat = Function::Multiply(Function::MakeTranslateMatrix(translate_), obj_->GetWorldMatrix());
    areaLight_.position = Function::TransformVM({ 0.0f,0.0f,0.0f }, worldMat);
    areaLight_.normal = -1.0f * YoshidaMath::GetForward(obj_->GetWorldMatrix());

    // プレイヤーのカメラ位置から
    Vector3 distance = playerCamera_->GetRay().origin - obj_->GetTranslate();
    float  length = Function::Length(distance);
    SEManager::SetVol(GetVol(length, 1.0f),SEManager::NOISE);

}

void VendingMac::Initialize()
{
    obj_->Initialize();
    SEManager::SoundPlay(SEManager::NOISE, true);
}

void VendingMac::Draw()
{
    obj_->Draw();
}

void VendingMac::CheckCollision()
{
    //自販機とrayの当たり判定
    if (OnCollisionRay()) {
        if (PlayerCommand::GetInstance()->Interact()) {

        }
    }
}

float VendingMac::GetVol(float length, float maxVol)
{
    if (length >= 100.0f) {
        return 0.0f;
    } else  if (length > 1.0f) {
        float vol = 1.0f / length;
        return  vol * maxVol;
    }

    return maxVol;
}

bool VendingMac::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), obj_->GetTranslate());
}

void VendingMac::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void VendingMac::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}
