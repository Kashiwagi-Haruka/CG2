#include "Key.h"
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <Model/ModelManager.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include"imgui.h"
#include"GameObject/SEManager/SEManager.h"

bool Key::isSendGetKeyMessage_ = false;
bool Key::isGetKey_ = false;
bool Key::isRayHit_ = false;
Key::Key()
{
    obj_ = std::make_unique<Object3d>();
    // モデルをセット
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/key", "key");
    obj_->SetModel("key");
    SetAABB({ .min = { -0.1f,-0.1f,-0.1f }, .max = { 0.1f,0.1f,0.1f } });
    SetCollisionAttribute(kCollisionKey);
    SetCollisionMask(kCollisionChair | kCollisionWall | kCollisionFloor);
}

void Key::Initialize()
{

    worldTransform_ = {
        .scale{2.0f, 2.0f, 2.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{-1.0f, 2.0f, -6.5f}
    };
    velocity_ = { 0.0f };
    obj_->Initialize();
    isRayHit_ = false;
    isLockerHit_ = false;
    isGetKey_ = false;
    isChairHit_ = false;
    isSendGetKeyMessage_ = false;
}

void Key::Update()
{
    isSendGetKeyMessage_ = false;
    obj_->SetEnableLighting(false);
    CheckCollision();

    //if (isGetKey_|| isLockerHit_|| isChairHit_) {
    //    velocity_.y = 0.0f;
    //} else {
    //    //重力処理
    //    const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
    //    velocity_.y -= YoshidaMath::kGravity * deltaTime;
    //    worldTransform_.translate += velocity_ * deltaTime;
    //}

    //y座標を固定する
    worldTransform_.translate.y = std::clamp(worldTransform_.translate.y, 0.0f, 2.4f);

    obj_->SetTransform(worldTransform_);
    obj_->Update();

#ifdef USE_IMGUI
    ImGui::Begin("Key");
    ImGui::DragFloat3("vel", &velocity_.x);
    ImGui::DragFloat3("translate", &worldTransform_.translate.x);
    ImGui::Checkbox("isGetKey", &isGetKey_);
    ImGui::End();
#endif
}

void Key::Draw()
{
    if (isGetKey_) {
        return;
    }
    obj_->Draw();
}

void Key::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Key::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

}

void Key::SetModel(const std::string& filePath)
{
    obj_->SetModel(filePath);
}

void Key::CheckCollision()
{
    isRayHit_ = false;

    if (isGetKey_) {
        return;
    }
    if (PlayerCommand::GetInstance()->InteractTrigger()) {
        isRayHit_ = OnCollisionRay();
        if (isRayHit_ && !PlayerCommand::GetIsGrab()) {
            isGetKey_ = true;
            isSendGetKeyMessage_ = true;
            SEManager::SoundPlay(SEManager::KEY);
        }
    }
    

}

bool Key::OnCollisionRay()
{
   return playerCamera_->OnCollisionRay(GetAABB(), worldTransform_.translate);

}

void Key::OnCollision(Collider* collider)
{

        isLockerHit_ = false;

        if (collider->GetCollisionAttribute() == kCollisionWall) {
            if (!isLockerHit_) {
                isLockerHit_ = true;
            }
        }

        if (collider->GetCollisionAttribute() == kCollisionChair) {
            if (!isChairHit_) {
                isChairHit_ = true;
            }
        }

        if (collider->GetCollisionAttribute() == kCollisionFloor) {

        }
    



}

Vector3 Key::GetWorldPosition() const
{
    return  worldTransform_.translate;
}
