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
    camera_->SetNearClip(0.2f);
    // Rayの設定
    ray_ = { .origin = {0.0f}, .diff = {0.0f} };

}

void PlayerCamera::Update() {
    SetHeadTransform();
    SetRay();
    const float fovY = Option::GetCurrentOptionData().fieldOfView * Function::kPi / 180.0f;
    camera_->SetFovY(fovY);
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

    if (ImGui::TreeNode("Eye Point Light")) {
        // カラー編集 (RGBA)
        ImGui::ColorEdit4("Color", &pointLight_.color.x);

        // 数値調整 (ドラッグ)
        ImGui::DragFloat("Intensity", &pointLight_.intensity, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Radius", &pointLight_.radius, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Decay", &pointLight_.decay, 0.01f, 0.0f, 10.0f);

        // 座標 (Vector3)
        ImGui::DragFloat3("Position", &pointLight_.position.x, 0.1f);

        // 有効/無効フラグ
        // bool型がintやint32_tで定義されている場合は (bool*)&pointLight_.shadowEnabled とキャストが必要な場合があります
        ImGui::Checkbox("Shadow Enabled", (bool*)&pointLight_.shadowEnabled);

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
    
    const float fovY = Option::GetCurrentOptionData().fieldOfView * Function::kPi / 180.0f;
    camera_->SetFovY(fovY);

    SetHeadTransform();
    SetTransform();
    SetRay();

    pointLight_.color = {1.0f,0.9f,0.9f,1.0f};
    pointLight_.decay = 1.3f;
    pointLight_.intensity = 1.0f;
    pointLight_.position = param_.transform.translate;
    pointLight_.radius = 2.0f;
    pointLight_.shadowEnabled = false;
}

bool PlayerCamera::OnCollisionRay(const AABB& localAABB, const Vector3& translate, const float min, const  float max)
{
    return YoshidaMath::RayIntersectsAABB(GetRay(), YoshidaMath::GetAABBWorldPos(localAABB, translate), min, max);
}


void PlayerCamera::SetHeadTransform()
{
    if (isFixedTransformEnabled_) {
        param_.transform = fixedTransform_;
        camera_->SetTransform(param_.transform);
        return;
    }

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

    //Playerの目
    param_.transform.translate = player_->GetJointWorldPos("CON.eyes");

    //ポイントライトの位置を代入する
    pointLight_.position = param_.transform.translate;

    //if (揺れなし) {
    //     param_.transform.translate = player_->GetTransform().translate;
    //     param_.transform.translate.y += 1.6f;
    //}



}

void PlayerCamera::SetTransform()
{
    camera_->SetTransform(param_.transform);
}

void PlayerCamera::EnableFixedTransform(const Transform& transform) {
    isFixedTransformEnabled_ = true;
    fixedTransform_ = transform;
    param_.transform = fixedTransform_;
    camera_->SetTransform(param_.transform);
}

void PlayerCamera::DisableFixedTransform() { isFixedTransformEnabled_ = false; }

Vector3 PlayerCamera::GetForward()
{
    return  YoshidaMath::GetForward(camera_->GetWorldMatrix());
}

