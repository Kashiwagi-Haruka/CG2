#include "Chair.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

PlayerCamera* Chair::playerCamera_ = nullptr;

Chair::Chair()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/chair", "chair");
    obj_->SetModel("chair");
    SetAABB({ .min = {-0.125f,0.0f,-0.125f},.max = {0.125f,0.5f,0.125f} });
    SetCollisionAttribute(kCollisionChair);
    SetCollisionMask(kCollisionPlayer | kCollisionFloor | kCollisionChair);
}

void Chair::OnCollision(Collider* collider)
{
    if (collider == this) {
        return;
    }

    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        Vector3 vel = playerCamera_->GetRay().diff*4.0f;
        vel.y = 0.0f;
        const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        transform_.translate += vel * deltaTime;
    }

    if (collider->GetCollisionAttribute() == kCollisionFloor) {
        velocity_.y = 0.0f;
    }


}

Vector3 Chair::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void Chair::Update()
{
    if (mirrorTransform_ != nullptr) {
        transform_.translate.x =- mirrorTransform_->translate.x;
        transform_.translate.y = mirrorTransform_->translate.y;
        transform_.translate.z = mirrorTransform_->translate.z;

        transform_.rotate.x = mirrorTransform_->rotate.x;
        transform_.rotate.y = -mirrorTransform_->rotate.y;
        transform_.rotate.z = mirrorTransform_->rotate.z;

        transform_.scale.x = mirrorTransform_->scale.x;
        transform_.scale.y = mirrorTransform_->scale.y;
        transform_.scale.z = mirrorTransform_->scale.z;
    }


    if (PlayerCommand::GetInstance()->Interact()) {
        if (OnCollisionRay()) {
            // カーソルに追従させて持ち上げる処理
            Vector3 origin = playerCamera_->GetTransform().translate;
            origin.y -= 0.5f;
            transform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
            transform_.translate.y = (std::max)(transform_.translate.y, 0.0f);
        }

    } else {
        const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        velocity_.y -= YoshidaMath::kGravity * deltaTime;
        transform_.translate += velocity_ * deltaTime;
    }


    transform_.translate.y = std::clamp(transform_.translate.y, 0.0f, 2.4f);
    YoshidaMath::ResolveCollision(transform_.translate, velocity_, GetCollisionInfo());

    obj_->SetTransform(transform_);
    obj_->Update();

}

void Chair::Initialize()
{
    obj_->Initialize();
    velocity_ = { 0.0f };
    transform_ = obj_->GetTransform();
}

void Chair::Draw()
{

    obj_->Draw();
}


bool Chair::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), transform_.translate);
}

void Chair::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Chair::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}
