#include "PlayerCamera.h"
#include"imgui.h"
#include"Function.h"
#include"GameObject/KeyBindConfig.h"
#include<algorithm>
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include"WinApp.h"
#include"GameObject/Player/Player.h"

PlayerCamera::PlayerCamera()
{
    //カメラの設定
    cameraTransform_ = {
    .scale{1.0f, 1.0f, 1.0f  },
    .rotate{0.0f, 0.0f, 0.0f  },
    .translate{0.0f, 0.0f, 0.0f}
    };

    //カメラを生成する
    camera_ = std::make_unique<Camera>();
    camera_->SetTransform(cameraTransform_);
    camera_->SetFovY(0.44f);
    //Rayの設定
    ray_ = { .origin = {0.0f},.diff = {0.0f} };

    //raySpriteの設定   
    raySprite_ = std::make_unique<RaySprite>();


}

void PlayerCamera::Update()
{
    SetTransform();
    SetRay();
    camera_->Update();
    raySprite_->Update();
}

void PlayerCamera::Rotate()
{
    Vector2 deltaRotate = PlayerCommand::GetInstance()->Rotate(eyeRotateSpeed_);
    player_->GetTransform().rotate.y += deltaRotate.y;
    cameraTransform_.rotate.x += deltaRotate.x;
#ifdef USE_IMGUI
    if (ImGui::TreeNode("Eye")) {
        ImGui::DragFloat("eyeRotateSpeed", &eyeRotateSpeed_, 0.1f, 0.1f);
        ImGui::DragFloat("eyeRotateX", &cameraTransform_.rotate.x, 0.1f);

        ImGui::DragFloat3("origin", &ray_.origin.x, 0.3f);
        ImGui::DragFloat3("diff", &ray_.diff.x, 0.3f);


        ImGui::TreePop();
    }
#endif
}

void PlayerCamera::SetRay()
{
    ray_.origin = cameraTransform_.translate;
    ray_.diff = GetForward();
}

void PlayerCamera::DrawRaySprite()
{
    raySprite_->Draw();
}

void PlayerCamera::Initialize()
{
    raySprite_->Initialize();
}

bool PlayerCamera::OnCollisionRay(const AABB& localAABB, const Vector3& translate)
{
    return YoshidaMath::RayIntersectsAABB(GetRay(), YoshidaMath::GetAABBWorldPos(localAABB, translate), kTMin_, kTMax_);
}


void PlayerCamera::SetTransform()
{

    Rotate();


    cameraTransform_.scale = { 1.0f,1.0f,1.0f };

    float halfPi = Function::kPi * 0.5f;
    cameraTransform_.rotate.x =
        std::clamp(
            cameraTransform_.rotate.x,
            -halfPi,
            halfPi - halfPi * 0.25f);

    cameraTransform_.rotate.y = player_->GetTransform().rotate.y;
    cameraTransform_.rotate.z = 0.0f;

    //Playerの頭
    cameraTransform_.translate = player_->GetJointWorldPos("Head");

    //if (揺れなし) {
    //    cameraTransform_.translate = player_->GetTransform().translate;
    //    cameraTransform_.translate.y += 1.6f;
    //}

    camera_->SetTransform(cameraTransform_);

}

Vector3 PlayerCamera::GetForward()
{
    return  YoshidaMath::GetForward(camera_->GetWorldMatrix());
}
