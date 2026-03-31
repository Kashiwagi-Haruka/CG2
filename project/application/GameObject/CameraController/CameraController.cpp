#define NOMINMAX
#include "CameraController.h"
#include"GameBase.h"
#include"GameObject/YoshidaMath/Easing.h"
#include<imgui.h>

CameraController::CameraController()
{    //プレイヤー視点のカメラ
    playerCamera_ = std::make_unique<PlayerCamera>();
    //デバックカメラ
    debugCamera_ = std::make_unique<DebugCamera>();
}

void CameraController::Initialize()
{
    useDebugCamera_ = false;
    isLerpEnd_ = false;
    isEventMode_ = false;
    targetPos_ = { 0.0f };
    targetRot_ = { 0.0f };
    lerpTime_ = 1.0f;
    currentLerp_ = 0.0f;
    
    eventCameraTransform_.scale = { 1.0f,1.0f,1.0f };

    playerCamera_->Initialize();
    //デバックカメラの設定
    debugCamera_->Initialize();
    debugCamera_->SetTranslation(playerCamera_->GetTransform().translate);
}

void CameraController::Update()
{

    if (useDebugCamera_) {
        //デバックカメラ
        debugCamera_->Update();
        //プレイヤーカメラの位置をセットする
        playerCamera_->GetCamera()->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());

        return;

    } else {

        if (isEventMode_) {

            currentLerp_ += GameBase::GetInstance()->GetDeltaTime() / lerpTime_;
            currentLerp_ = std::min(currentLerp_, 1.0f);

            if (currentLerp_ == 1.0f) {
                if (!isLerpEnd_) {
                    isLerpEnd_ = true;
                }
            }

            playerCamera_->GetCamera()->SetTranslate(YoshidaMath::Easing::Lerp(eventCameraTransform_.translate, targetPos_, currentLerp_));
            playerCamera_->GetCamera()->SetRotate(YoshidaMath::Easing::Lerp(eventCameraTransform_.rotate, targetRot_, currentLerp_));
        
 
        } else {

            FollowPlayer();
        }

        playerCamera_->Update();
    }

#ifdef USE_IMGUI
    if (ImGui::Begin("Camera")) {
        ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
        ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
        if (ImGui::TreeNode("Transform")) {

            if (!useDebugCamera_) {
                auto& playerCameraT = playerCamera_->GetTransform();
                ImGui::DragFloat3("Scale", &playerCameraT.scale.x, 0.01f);
                ImGui::DragFloat3("Rotate", &playerCameraT.rotate.x, 0.01f);
                ImGui::DragFloat3("Translate", &playerCameraT.translate.x, 0.01f);
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

#endif
}

void CameraController::SetPlayerCameraTransformToEventCameraTransform()
{    // ★ 追加：プレイヤーカメラの位置を最新化
    playerCamera_->SetHeadTransform();
    eventCameraTransform_ = playerCamera_->GetTransform();
}

void CameraController::SetPlayerCameraTransformToTarget()
{
    // ★ 追加：プレイヤーカメラの位置を最新化
    playerCamera_->SetHeadTransform();
    Transform transform = playerCamera_->GetTransform();
    targetPos_ = transform.translate;
    targetRot_ = transform.rotate;
}

void CameraController::SetCameraTransform(const Vector3& pos, const Vector3& rot)
{
    eventCameraTransform_.scale = { 1.0f,1.0f,1.0f };
    eventCameraTransform_.translate = pos;
    eventCameraTransform_.rotate = rot;
}

void CameraController::FollowPlayer()
{
    playerCamera_->SetHeadTransform();
    playerCamera_->SetTransform();
}
