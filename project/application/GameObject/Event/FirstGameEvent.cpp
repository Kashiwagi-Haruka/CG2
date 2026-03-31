#include "FirstGameEvent.h"
#include"GameBase.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/CameraController/CameraController.h"

void FirstGameEvent::Update()
{
    if (!IsRunning()) {
        return;
    }

    if (PlayerCommand::GetInstance()->EvetSkipTrigger()) {
        triggerCount_++;
    }

    auto* cameraController = CameraController::GetInstance();

    if (cameraController->GetInstance()->IsLerpEnd()) {

        if (count_ < 3) {

            Transform playerTransform = cameraController->GetPlayerCamera()->GetTransform();

            const  float qPi = Function::kPi * 0.125f;
            const float rotateX = 0.75f;
            Vector3 offsetTransform = { 0.0f,-0.25f,-0.5f };
            if (count_ == 0) {
                Vector3 offset = { rotateX,qPi,0.0f };
                cameraController->MoveTo(playerTransform.translate, playerTransform.rotate, playerTransform.translate + offsetTransform, playerTransform.rotate + offset, 1.25f);
            } else if (count_ == 1) {
                Vector3 offsetX = { rotateX,0.0f,0.0f };
                Vector3 offsetY = { 0.0f,qPi,0.0f };
                cameraController->MoveTo(playerTransform.translate + offsetTransform, playerTransform.rotate + offsetX + offsetY, playerTransform.translate + offsetTransform, playerTransform.rotate + offsetX - offsetY, 1.0f);
            } else if (count_ == 2) {
                Vector3 offset = { rotateX,-qPi,0.0f };
                cameraController->MoveTo(playerTransform.translate + offsetTransform, playerTransform.rotate + offset, playerTransform.translate, playerTransform.rotate, 1.25f);
            }
        }
        count_++;
    }

    if (count_ >= 4 || triggerCount_ >= 2) {
        EndEvent();
    }

}

void FirstGameEvent::OnStart()
{
    count_ = 0;
    triggerCount_ = 0;
    auto* cameraController = CameraController::GetInstance();
    cameraController->SetEventMode(true);
    cameraController->MoveTo({ 6.0f,1.0f,-6.0f }, { 0.0f,Function::kPi,0.0f }, { 8.0f,1.0f,-6.0f }, { 0.0f,Function::kPi,0.0f }, 5.0f);

}

void FirstGameEvent::OnEnd()
{
    // イベント終了時に通常カメラへ戻す
    CameraController::GetInstance()->SetEventMode(false);
}
