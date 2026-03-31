#define NOMINMAX
#include "CameraController.h"
#include"GameBase.h"
#include"GameObject/YoshidaMath/Easing.h"

CameraController::CameraController()
{    //プレイヤー視点のカメラ
    playerCamera_ = std::make_unique<PlayerCamera>();
    //デバックカメラ
    debugCamera_ = std::make_unique<DebugCamera>();
}

void CameraController::Initialize()
{
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

        if (!isEventMode_) {
            FollowPlayer();
            return;
        }

        currentLerp_ += GameBase::GetInstance()->GetDeltaTime() / lerpTime_;
        currentLerp_ = std::min(currentLerp_, 1.0f);

        eventCameraTransform_.translate = YoshidaMath::Easing::Lerp(eventCameraTransform_.translate, targetPos_, currentLerp_);
        eventCameraTransform_.rotate = YoshidaMath::Easing::Lerp(eventCameraTransform_.rotate, targetRot_, currentLerp_);

        playerCamera_->GetCamera()->SetTransform(eventCameraTransform_);
    }
}

void CameraController::AppendPlayerCameraTransformToEventCameraTransform()
{
    eventCameraTransform_ = playerCamera_->GetTransform();;
}

void CameraController::SetCameraTransform(const Vector3& pos, const Vector3& rot)
{
    eventCameraTransform_.scale = { 1.0f,1.0f,1.0f };
    eventCameraTransform_.translate = pos;
    eventCameraTransform_.rotate = rot;
}

void CameraController::FollowPlayer()
{
    playerCamera_->Update();

}
