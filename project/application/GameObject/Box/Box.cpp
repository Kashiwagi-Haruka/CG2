#include "Box.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include"application/Color/Color.h"
PlayerCamera* Box::playerCamera_ = nullptr;
namespace {
const Vector4 kRayHitOutlineColor = {1.0f, 1.0f, 0.0f, 1.0f};
const float kRayHitOutlineWidth = 1.0f;
} // namespace
Box::Box()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/box", "box");
    obj_->SetModel("box");
    SetAABB({ .min = {-0.25f,-0.125f,-0.25f},.max = {0.25f,0.125f,0.25f} });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionPlayer | kCollisionFloor | kCollisionChair | kCollisionKey | kCollisionWall);
}

void Box::OnCollision(Collider* collider)
{
  /*  if (collider->GetCollisionAttribute() == kCollisionWall|| collider->GetCollisionAttribute() == kCollisionFloor) {
        obj_->SetColor(COLOR::RED);
    } else {
        obj_->SetColor(COLOR::WHITE);
    }*/

}

Vector3 Box::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void Box::Update() {
	Mirror();

	const bool isRayHitBeforeMove = OnCollisionRay();
	// インタラクトをトリガーすると
	if (PlayerCommand::GetInstance()->InteractTrigger()) {

		if (isGrab_) {
			// 持っていたら離す
			isGrab_ = false;
			// プレイヤーの状態をセットする
			PlayerCommand::SetIsGrab(false);
		} else {
			// rayと重なる
			if (isRayHitBeforeMove) {
				if (!PlayerCommand::GetIsGrab()) {
					isGrab_ = true;
					// プレイヤーの状態をセットする
					PlayerCommand::SetIsGrab(true);
				}
			}
		}
	}

	Grab();
	isRayHit_ = OnCollisionRay();

	obj_->SetTransform(transform_);
	obj_->Update();
}

void Box::Initialize() {
	isRayHit_ = false;
	obj_->Initialize();
	velocity_ = {0.0f};
	transform_ = obj_->GetTransform();
	obj_->SetOutlineColor(kRayHitOutlineColor);
	obj_->SetOutlineWidth(kRayHitOutlineWidth);
}

void Box::Draw() {
	if (isRayHit_) {
		Object3dCommon::GetInstance()->DrawCommonOutline();
	} else {
		Object3dCommon::GetInstance()->DrawCommon();
	}
	obj_->Draw();
}

void Box::Mirror()
{
    if (mirrorTransform_ != nullptr) {
        transform_.translate.x = -mirrorTransform_->translate.x;
        transform_.translate.y = mirrorTransform_->translate.y;
        transform_.translate.z = mirrorTransform_->translate.z;

        transform_.rotate.x = mirrorTransform_->rotate.x;
        transform_.rotate.y = -mirrorTransform_->rotate.y;
        transform_.rotate.z = mirrorTransform_->rotate.z;

        transform_.scale.x = mirrorTransform_->scale.x;
        transform_.scale.y = mirrorTransform_->scale.y;
        transform_.scale.z = mirrorTransform_->scale.z;
    }
}

void Box::Grab()
{
    if (isGrab_ && PlayerCommand::GetIsGrab()) {
        // カーソルに追従させて持ち上げる処理
        Vector3 origin = playerCamera_->GetTransform().translate;
        origin.y -= 0.25f;
        transform_.translate = origin + playerCamera_->GetRay().diff;
        transform_.translate.y = (std::max)(transform_.translate.y, 0.0f);
        velocity_.y = 0.0f;
        transform_.rotate.y = playerCamera_->GetTransform().rotate.y;
    } else {
        const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        velocity_.y -= YoshidaMath::kGravity * deltaTime*0.5f;
        transform_.translate += velocity_ * deltaTime;
    }

    transform_.translate.y = std::clamp(transform_.translate.y, GetAABB().max.y, 2.4f);
    YoshidaMath::ResolveCollision(transform_.translate, velocity_, GetCollisionInfo());
}

bool Box::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), transform_.translate);
   
}

void Box::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Box::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}
