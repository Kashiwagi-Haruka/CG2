#define NOMINMAX
#include "Chair.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/SEManager/SEManager.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "Text/ChairMenu/ChairMenu.h"

PlayerCamera* Chair::playerCamera_ = nullptr;

Chair::Chair() {
	obj_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/chair", "chair");
	obj_->SetModel("chair");
	SetAABB({
	    .min = {-0.125f, 0.0f, -0.125f},
          .max = {0.125f,  0.5f, 0.125f }
    });
	SetCollisionAttribute(kCollisionChair);
	SetCollisionMask(kCollisionPlayer | kCollisionFloor | kCollisionChair | kCollisionKey | kCollisionWall);
}

void Chair::OnCollision(Collider* collider) {
	if (collider == this) {
		return;
	}

	if (collider->GetCollisionAttribute() == kCollisionFloor || collider->GetCollisionAttribute() == kCollisionKey || collider->GetCollisionAttribute() == kCollisionPlayer) {
		velocity_.y = 0.0f;
	}
}

Vector3 Chair::GetWorldPosition() const { return obj_->GetTranslate(); }

void Chair::Update() {
	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	chairMoveSeTimer_ = std::max(0.0f, chairMoveSeTimer_ - deltaTime);

	Mirror();

	// Rayが外れたらメニューを自動で閉じる
	if (isPreOnCollisionRay_ && !OnCollisionRay() && !isGrab_ && ChairMenu::GetIsShowMenu()) {
		ChairMenu::SetIsShowMenu(false);
		ChairMenu::SetIsSelectButton(false);
	}

	// rayと重なる、または椅子を持っていると
	if (OnCollisionRay() || isGrab_) {
		// インタラクトをトリガーすると
		if (PlayerCommand::GetInstance()->InteractTrigger()) {
			if (ChairMenu::GetIsShowMenu()) {
				if (ChairMenu::GetIsSelectButton()) {
					// 数値によって処理を変更する
					SwichCommand();
					ChairMenu::SetIsSelectButton(false);
				}
			} else {
				// メニューを表示してないとき表示する
				ChairMenu::SetIsShowMenu(true);
			}
		}
	}

	isPreOnCollisionRay_ = OnCollisionRay();

	Grab();

	obj_->SetTransform(transform_);
	obj_->Update();
}

void Chair::Initialize() {
	isStand_ = false;
	obj_->Initialize();
	velocity_ = {0.0f};
	transform_ = obj_->GetTransform();
	chairMoveSeTimer_ = 0.0f;
}

void Chair::Draw()
{
    obj_->Draw();
}

void Chair::SwichCommand()
{
    switch (ChairMenu::GetSelectButtonNum())
    {
    case ChairMenu::GRAB_TEXT:
        //メニューを表示しているとき
        if (isGrab_) {
            //持っていたら離す
            isGrab_ = false;
            //プレイヤーの状態をセットする
            PlayerCommand::SetIsGrab(false);
            //グラブ終了後メニューを閉じる
            ChairMenu::SetIsShowMenu(false);
        } else {
            if (!PlayerCommand::GetIsGrab()) {
                if (OnCollisionRay()) {
                    isGrab_ = true;
                    //プレイヤーの状態をセットする
                    PlayerCommand::SetIsGrab(true);
                }

            }
        }


        break;
    case ChairMenu::STAND_TEXT:

        if (!PlayerCommand::GetIsStand()) {
            PlayerCommand::SetIsStand(true);
            isStand_ = true;
        }

        //メニューを閉じる
        ChairMenu::SetIsShowMenu(false);
        break;
    default:
        //メニューを閉じる
        ChairMenu::SetIsShowMenu(false);
        break;
    }


}

void Chair::Mirror() {
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

void Chair::Grab() {
	const Vector3 previousPosition = transform_.translate;
	static constexpr float kChairMoveSEInterval = 0.25f;
	static constexpr float kMoveThreshold = 0.01f;

	if (isGrab_ && PlayerCommand::GetIsGrab()) {
		// カーソルに追従させて持ち上げる処理
		Vector3 origin = playerCamera_->GetTransform().translate;
		origin.y -= 1.5f;
		transform_.translate = origin + playerCamera_->GetRay().diff * 1.5f;
		transform_.translate.y = (std::max)(transform_.translate.y, 0.0f);
		velocity_.y = 0.0f;
		transform_.rotate.y = playerCamera_->GetTransform().rotate.y;

		const float moveDeltaX = transform_.translate.x - previousPosition.x;
		const float moveDeltaZ = transform_.translate.z - previousPosition.z;
		const bool isMovingWhileGrabbed = (moveDeltaX * moveDeltaX + moveDeltaZ * moveDeltaZ) > (kMoveThreshold * kMoveThreshold);
		if (isMovingWhileGrabbed && chairMoveSeTimer_ <= 0.0f) {
			SEManager::SoundPlay(SEManager::CHAIR);
			chairMoveSeTimer_ = kChairMoveSEInterval;
		}

	} else {
		const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
		velocity_.y -= YoshidaMath::kGravity * deltaTime;
		transform_.translate += velocity_ * deltaTime;
	}

	transform_.translate.y = std::clamp(transform_.translate.y, 0.0f, 2.4f);
	YoshidaMath::ResolveCollision(transform_.translate, velocity_, GetCollisionInfo());
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
