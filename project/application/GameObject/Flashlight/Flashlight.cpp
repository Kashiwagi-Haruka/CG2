#include "Flashlight.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"
#include<imgui.h>

bool Flashlight::isSendGetLightMessage_ = false;

Flashlight::Flashlight()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/light", "light");
    obj_->SetModel("light");
    SetAABB({ .min = {-0.1f,-0.1f,-0.1f},.max = {0.1f,0.1f,0.1f} });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionPlayer);
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

    if (isGetLight_) {

        transform_.translate = { 0.0f,0.1f,0.0f };
        transform_.rotate = { 0.0f,Function::kPi * 0.5f,0.0f };
        transform_.scale = { 1.0f,1.0f,1.0f };

        Matrix4x4  handMat =player_->GetJointMatrix("Hand.L");
        Matrix4x4 child = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
        child = Function::Multiply(child, handMat);
        obj_->SetWorldMatrix(child);
#ifdef USE_IMGUI
        ImGui::Begin("FlashLight");
        ImGui::DragFloat3("direction", &spotLight_.direction.x);
        ImGui::Text("translate %f %f %f", handMat.m[3][0], handMat.m[3][1], handMat.m[3][2]);

        ImGui::End();

#endif
    } else {
        //y座標を固定する
        transform_.rotate = { 0.0f,0.0f,0.0f };
        transform_.translate.y = std::clamp(transform_.translate.y, 0.0f, 2.4f);
        obj_->SetTransform(transform_);
    }


    obj_->Update();
   
    UpdateSpotLight();
   
}

void Flashlight::Initialize()
{
    isGetLight_ = false;
    isSendGetLightMessage_ = false;

    obj_->Initialize();
    transform_.translate = {4.0f,0.1f,5.0f};
    transform_.rotate = { 0.0f,0.0f,0.0f };
    transform_.scale = { 1.0f,1.0f,1.0f };
    SetLight();
}

void Flashlight::Draw()
{
 
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

void Flashlight::CheckCollision()
{

    if (PlayerCommand::GetInstance()->InteractTrigger()) {
        //keyとrayの当たり判定
        if (Player::GetIsGrab()) {
            isGetLight_ = false;
            Player::SetIsGrab(false);
            transform_.translate =  GetWorldPosition();
        } else {
            if (OnCollisionRay()) {
                isGetLight_ = true;
                Player::SetIsGrab(true);
                isSendGetLightMessage_ = true;

            }

        }

    }


}

void Flashlight::UpdateSpotLight()
{
    spotLight_.position = GetWorldPosition();
    spotLight_.direction = -YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 10.0f;

}

bool Flashlight::OnCollisionRay()
{
     return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
