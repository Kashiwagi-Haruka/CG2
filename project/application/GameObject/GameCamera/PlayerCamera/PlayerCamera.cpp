#include "PlayerCamera.h"
#include"imgui.h"
#include"Function.h"
#include"GameObject/KeyBindConfig.h"
#include<algorithm>
#include"GameObject/YoshidaMath/YoshidaMath.h"


PlayerCamera::PlayerCamera()
{
    //カメラの設定
    cameraTransform_ = {
    .scale{1.0f, 1.0f, 1.0f  },
    .rotate{0.0f, 0.0f, 0.0f  },
    .translate{0.0f, 5.0f, -10.0f}
    };

    //カメラを生成する
    camera_ = std::make_unique<Camera>();
    camera_->SetTransform(cameraTransform_);
    //Rayの設定
    ray_ = { .origin = {0.0f},.diff = {0.0f} };
}

void PlayerCamera::Update()
{
    SetTransform();
    SetRay();
    camera_->Update();
}

void PlayerCamera::Rotate()
{
    Vector2 deltaRotate = PlayerCommand::GetInstance()->Rotate(eyeRotateSpeed_);

    playerTransform_->rotate.y += deltaRotate.y;
    cameraTransform_.rotate.x += deltaRotate.x;
#ifdef USE_IMGUI
    if (ImGui::TreeNode("Eye")) {
        ImGui::DragFloat("eyeRotateSpeed", &eyeRotateSpeed_, 0.1f, 0.1f);
        ImGui::DragFloat("eyeRotateX", &cameraTransform_.rotate.x, 0.1f);

        ImGui::DragFloat3("origin", &ray_.origin.x, 0.3f);
        ImGui::DragFloat3("diff", &ray_.diff.x, 0.3f);
		ImGui::TreePop();
        ImGui::End();
	}
#endif
        
    
}

void PlayerCamera::SetRay()
{
    ray_.origin = cameraTransform_.translate;
    ray_.diff = GetForward();
}

void PlayerCamera::SetTransform()
{
    assert(playerTransform_);

    Rotate();

    //Playerからの視点
    cameraTransform_.scale = { 1.0f,1.0f,1.0f };
    float halfPi = Function::kPi * 0.5f;
    cameraTransform_.rotate.x =

        std::clamp(
            cameraTransform_.rotate.x,
            -halfPi,
            halfPi);

    cameraTransform_.rotate.y = playerTransform_->rotate.y;
    cameraTransform_.rotate.z = 0.0f;

    cameraTransform_.translate = playerTransform_->translate;
    cameraTransform_.translate.y += 1.5f;
    camera_->SetTransform(cameraTransform_);

}

Vector3 PlayerCamera::GetForward()
{
    return  YoshidaMath::GetForward(camera_->GetWorldMatrix());
}
