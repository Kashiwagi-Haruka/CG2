#include "PlayerCamera.h"
#include"imgui.h"
#include"Function.h"
#include"GameObject/KeyBindConfig.h"
#include<algorithm>
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include"WinApp.h"
#include"GameObject/Player/Player.h"
#include "application/Option/Option.h"
#include"Option/OptionData.h"

PlayerCamera::PlayerCamera() {

    // カメラを生成する
    camera_ = std::make_unique<Camera>();
    camera_->SetFovY(0.44f);
    // Rayの設定
    ray_ = { .origin = {0.0f}, .diff = {0.0f} };

}

void PlayerCamera::Update() {
    SetRay();
    SetHeadTransform();
    camera_->Update();
}

void PlayerCamera::Rotate() {
    auto& optionData = Option::GetCurrentOptionData();
    const Vector2 optionCameraMoveSpeed = optionData.CameraMoveSpeed;
    Vector2 deltaRotate = PlayerCommand::GetInstance()->Rotate(rotateSpeed_, optionData.isFlipHorizontally, optionData.isFlipVertically);
    player_->GetTransform().rotate.y += deltaRotate.y * optionCameraMoveSpeed.y;
    param_.transform.rotate.x += deltaRotate.x * optionCameraMoveSpeed.x;
#ifdef USE_IMGUI
    if (ImGui::TreeNode("Eye")) {
        ImGui::Text("rotateSpeed : %f", rotateSpeed_);
        ImGui::Text("deltaRotate x: %f,y: %f", deltaRotate.x, deltaRotate.y);
        ImGui::DragFloat("eyeRotateX", &param_.transform.rotate.x, 0.1f);

        ImGui::DragFloat3("origin", &ray_.origin.x, 0.3f);
        ImGui::DragFloat3("diff", &ray_.diff.x, 0.3f);

        ImGui::TreePop();
    }
#endif
}

void PlayerCamera::SetRay()
{
    ray_.origin = param_.transform.translate;
    ray_.diff = GetForward();
}


void PlayerCamera::Initialize()
{
    auto& gaveSave = GameSave::GetInstance();
   
    if (!gaveSave.GetInitStart()) {
        param_ = gaveSave.GetCameraSaveData();
    } else {
        param_.transform = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
    }

    SetHeadTransform();
    SetTransform();
    SetRay();
}

bool PlayerCamera::OnCollisionRay(const AABB& localAABB, const Vector3& translate)
{
    return YoshidaMath::RayIntersectsAABB(GetRay(), YoshidaMath::GetAABBWorldPos(localAABB, translate), kTMin_, kTMax_);
}


void PlayerCamera::SetHeadTransform()
{

    Rotate();
    param_.transform.scale = { 1.0f,1.0f,1.0f };

    float halfPi = Function::kPi * 0.5f;
    param_.transform.rotate.x =
        std::clamp(
            param_.transform.rotate.x,
            -halfPi,
            halfPi - halfPi * 0.25f);

    param_.transform.rotate.y = player_->GetTransform().rotate.y;
    param_.transform.rotate.z = 0.0f;

    //Playerの頭
    param_.transform.translate = player_->GetJointWorldPos("Head");

    //if (揺れなし) {
    //     param_.transform.translate = player_->GetTransform().translate;
    //     param_.transform.translate.y += 1.6f;
    //}



}

void PlayerCamera::SetTransform()
{
    camera_->SetTransform(param_.transform);
}

Vector3 PlayerCamera::GetForward()
{
    return  YoshidaMath::GetForward(camera_->GetWorldMatrix());
}


