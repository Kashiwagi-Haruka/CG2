#include "SeveredHand.h"
#include <Model/ModelManager.h>
#include <GameObject/KeyBindConfig.h>
#include"Object3d/Object3dCommon.h"
#include"imgui.h"
#include"GameObject/SEManager/SEManager.h"
#include"GameSave/GameSave.h"
#include"GameBase.h"

namespace {
    const Vector4 kRayHitOutlineColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    const float kRayHitOutlineWidth = 10.0f;
} // namespace

SeveredHand::SeveredHand()
{
    obj_ = std::make_unique<Object3d>();
    // モデルをセット
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Severed_Hand", "Severed_Hand");
    obj_->SetModel("Severed_Hand");
    SetAABB({ .min = { -0.1f,-0.1f,-0.1f }, .max = { 0.1f,0.1f,0.1f } });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionChair | kCollisionWall | kCollisionFloor);
}

void SeveredHand::Initialize()
{
    obj_->Initialize();
    obj_->RegisterEditor("Severed_Hand");
    obj_->SetOutlineColor(kRayHitOutlineColor);
    obj_->SetOutlineWidth(kRayHitOutlineWidth);

    isRayHit_ = false;
    isLook_ = false;
    worldPos_ = { 0.0f };
}

void SeveredHand::Update()
{
    CheckCollision();

    Matrix4x4 child = Function::MakeAffineMatrix(obj_->GetScale(), obj_->GetRotate(), obj_->GetTranslate());

    if (parentMat_) {
        child = Function::Multiply(child, *parentMat_);
    }

    obj_->SetWorldMatrix(child);
    obj_->Update();

    worldPos_ = YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());

#ifdef USE_IMGUI
    ImGui::Begin("SeveredHand");
    ImGui::Text("transform %f %f %f", obj_->GetTransform().translate.x,obj_->GetTransform().translate.y, obj_->GetTransform().translate.z);
    ImGui::DragFloat3("worldPos", &worldPos_.x);
    ImGui::Checkbox("isLook_", &isLook_);

    ImGui::End();
#endif
}

void SeveredHand::Draw() {

    if (isRayHit_) {
        Object3dCommon::GetInstance()->DrawCommon();
        obj_->Draw();
        Object3dCommon::GetInstance()->DrawCommonOutline();
        obj_->Draw();
        Object3dCommon::GetInstance()->EndOutlineDraw();
    } else {
        Object3dCommon::GetInstance()->DrawCommon();
        obj_->Draw();
    }
}
void SeveredHand::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void SeveredHand::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

}

void SeveredHand::CheckCollision() {

    isRayHit_ = OnCollisionRay();

    if (isRayHit_ && !PlayerCommand::GetIsGrab()) {
        if (!isLook_) {
            isLook_ = true;
            SEManager::SoundPlay(SEManager::KEY);
       }
  
    }

}

bool SeveredHand::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), worldPos_);

}

void SeveredHand::OnCollision(Collider* collider)
{

    if (collider->GetCollisionAttribute() == kCollisionWall) {
    }

    if (collider->GetCollisionAttribute() == kCollisionChair) {

    }

    if (collider->GetCollisionAttribute() == kCollisionFloor) {

    }

}

Vector3 SeveredHand::GetWorldPosition() const
{

    return  worldPos_;
}
