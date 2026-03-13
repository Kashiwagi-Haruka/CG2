#include "VendingMac.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"

VendingMac::VendingMac()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/vendingMac", "vendingMac");
    obj_->SetModel("vendingMac");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,1.83f,0.5f} });
    SetCollisionAttribute(kCollisionVendingMac);
    SetCollisionMask(kCollisionPlayer);
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
}

void VendingMac::Initialize()
{
    obj_->Initialize();
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
