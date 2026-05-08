#include "Flashlight.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include<imgui.h>
#include"GameObject/SEManager/SEManager.h"
#include "Engine/Texture/Mesh/Object3d/Object3dCommon.h"

bool Flashlight::isSendGetLightMessage_ = false;
bool Flashlight::isGetLight_ = false;
bool Flashlight::isRayHit_ = false;
ProgressSaveData* Flashlight::progressSaveData_ = nullptr;
Flashlight::Flashlight()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/light", "light");
    obj_->SetModel("light");
    SetAABB({ .min = {-0.1f,-0.1f,-0.1f},.max = {0.1f,0.1f,0.1f} });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionPlayer | kCollisionFloor);
    spotLight_.color = { 1.0f, 1.0f, 0.5f, 1.0f };
    spotLight_.position = obj_->GetTranslate();
    spotLight_.direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 2.0f;
    spotLight_.distance = 10.0f;
    spotLight_.decay = 2.0f;
    spotLight_.cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLight_.cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
}

void Flashlight::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
    }
}

Vector3 Flashlight::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Flashlight::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Flashlight::Update()
{
    isSendGetLightMessage_ = false;

    CheckCollision();



#ifdef USE_IMGUI
    ImGui::Begin("FlashLight");
    ImGui::DragFloat3("direction", &spotLight_.direction.x);
    ImGui::Checkbox("isGet", &isGetLight_);
    //ImGui::Text("translate %f %f %f", handMat.m[3][0], handMat.m[3][1], handMat.m[3][2]);
    ImGui::End();

#endif



    if (isGetLight_) {

        transform_.translate = { 0.0f,0.1f,0.0f };
        transform_.rotate = { 0.0f,Function::kPi * 0.5f,0.0f };
        transform_.scale = { 1.0f,1.0f,1.0f };

        Matrix4x4  handMat = player_->GetJointMatrix("Hand.L");
        Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
        child = Function::Multiply(child, handMat);
        obj_->SetWorldMatrix(child);

    } else {
        transform_ =  obj_->GetTransform();
        //y座標を固定する
        transform_.translate.y = std::clamp(transform_.translate.y, 0.0f, 2.4f);
        obj_->SetTransform(transform_);
    }


    obj_->Update();

    UpdateSpotLight();

}

void Flashlight::Initialize()
{
    assert(progressSaveData_);

    isGetLight_ = progressSaveData_->isLightHave;
    isLightOn_ = true;

    isSendGetLightMessage_ = false;
    isRayHit_ = false;
    obj_->Initialize();
    obj_->RegisterEditor("Flashlight");
    SetLight();
	obj_->SetOutlineColor({1.0f, 1.0f, 0.0f, 1.0f});
	obj_->SetOutlineWidth(10.0f);
}

void Flashlight::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();   
    obj_->Draw();
	if (isRayHit_) {
	Object3dCommon::GetInstance()->DrawCommonOutline();
	obj_->Draw();
	Object3dCommon::GetInstance()->EndOutlineDraw();
    }
}

void Flashlight::SetLight()
{
    spotLight_.color = { 1.0f, 1.0f, 0.5f, 1.0f };
    spotLight_.position = obj_->GetTranslate();
    spotLight_.direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 1.0f;
    spotLight_.distance = 15.0f;
    spotLight_.decay = 2.0f;
    spotLight_.cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLight_.cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
}

void Flashlight::CheckCollision()
{
    isRayHit_ = OnCollisionRay();

    if (PlayerCommand::GetInstance()->InteractTrigger()) {
        //keyとrayの当たり判定

        if (!PlayerCommand::GetIsGrab()) {
            if (isRayHit_) {
                isGetLight_ = true;
                /*          PlayerCommand::SetIsGrab(true);*/
                isSendGetLightMessage_ = true;
                assert(progressSaveData_);
                //記録する
                progressSaveData_->isLightHave = isGetLight_;
            }

        }

    }


}

void Flashlight::UpdateSpotLight()
{
    spotLight_.position = GetWorldPosition();
    spotLight_.direction = -YoshidaMath::GetForward(obj_->GetWorldMatrix());
  
    if (!isGetLight_) {
        return;
    }

    if (PlayerCommand::GetInstance()->SwitchLight()) {
        isLightOn_ = !isLightOn_;
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
        if (isLightOn_) {
            spotLight_.intensity = 1.0f;
        } else {
            spotLight_.intensity = 0.0f;
        }
    }

}

bool Flashlight::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
