#include "Key.h"
#include <Model/ModelManager.h>
#include <GameObject/KeyBindConfig.h>
#include"Object3d/Object3dCommon.h"
#include"imgui.h"
#include"GameObject/SEManager/SEManager.h"
#include"GameSave/GameSave.h"
bool Key::isSendGetKeyMessage_ = false;
bool Key::isGetKey_ = false;
bool Key::isRayHit_ = false;
namespace {
const Vector4 kRayHitOutlineColor = {1.0f, 1.0f, 0.0f, 1.0f};
const float kRayHitOutlineWidth = 10.0f;
} // namespace
Key::Key()
{
    obj_ = std::make_unique<Object3d>();
    // モデルをセット
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/key", "key");
    obj_->SetModel("key");
	obj_->SetOutlineColor(kRayHitOutlineColor);
	obj_->SetOutlineWidth(kRayHitOutlineWidth);
    SetAABB({ .min = { -0.1f,-0.1f,-0.1f }, .max = { 0.1f,0.1f,0.1f } });
    SetCollisionAttribute(kCollisionKey);
    SetCollisionMask(kCollisionChair | kCollisionWall | kCollisionFloor);
}

void Key::Initialize()
{

    worldTransform_ = {
        .scale{2.0f, 2.0f, 2.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{-1.0f, 2.0f, -6.0f}
    };


    //worldTransform_ = obj_->GetTransform();


    velocity_ = { 0.0f };
    obj_->Initialize();
    obj_->RegisterEditor("Key");
    isRayHit_ = false;
    isLockerHit_ = false;

    auto& gameSave  = GameSave::GetInstance();

    if (!gameSave.GetInitStart()) {
        isGetKey_ = gameSave.GetProgressSaveData().isKeyHave;
    } else {
        isGetKey_ = false;
    }


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

void Key::Draw() {
	if (isGetKey_) {
		return;
	}
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

void Key::CheckCollision() {
	if (isGetKey_) {
		isRayHit_ = false;
		return;
	}

	isRayHit_ = OnCollisionRay();

	if (PlayerCommand::GetInstance()->InteractTrigger()) {
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
