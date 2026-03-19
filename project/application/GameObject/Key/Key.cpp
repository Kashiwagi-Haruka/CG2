#include "Key.h"
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <Model/ModelManager.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>
#include"GameObject/Player/Player.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

bool Key::isSendGetKeyMessage_ = false;

Key::Key()
{
    obj_ = std::make_unique<Object3d>();
    // モデルをセット
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/key", "key");
    obj_->SetModel("key");

}

void Key::Initialize()
{
    worldTransform_ = {
        .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{-5.0f, 0.0f, 0.0f}
    };

    obj_->Initialize();
    //上に伸びてる
    SetAABB({ .min = { -0.25f,0.0f,-0.25f }, .max = { 0.25f,0.25f,0.25f } });
    SetCollisionAttribute(kCollisionKey);
    SetCollisionMask(kCollisionDoor | kCollisionChair);

    isChairHit_ = false;
    isSendGetKeyMessage_ = false;
}

void Key::Update()
{
    isSendGetKeyMessage_ = false;
    obj_->SetTransform(worldTransform_);
    obj_->Update();
}

void Key::Draw()
{
    obj_->Draw();
}

void Key::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Key::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

}

void Key::SetModel(const std::string& filePath)
{
    obj_->SetModel(filePath);
}

void Key::CheckCollision()
{

    if (PlayerCommand::GetInstance()->InteractTrigger()) {
        //keyとrayの当たり判定
        if (Player::GetIsGrab() && isGetKey_) {
            isGetKey_ = false;
            Player::SetIsGrab(false);
        } else {
            if (OnCollisionRay()) {
                isGetKey_ = true;
                Player::SetIsGrab(true);
                isSendGetKeyMessage_ = true;
            }
        
        }
  
    }

    if (isGetKey_) {
        // カーソルに追従させて持ち上げる処理
        Vector3 origin = playerCamera_->GetTransform().translate;
        worldTransform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
        worldTransform_.translate.y = (std::max)(worldTransform_.translate.y, 0.0f);
        velocity_.y = 0.0f;
    } else {
        //重力処理
        const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        velocity_.y -= YoshidaMath::kGravity * deltaTime * mass_;
        worldTransform_.translate += velocity_ * deltaTime;

    }
    //押し戻し処理
    YoshidaMath::ResolveCollision(worldTransform_.translate, velocity_, GetCollisionInfo());
    //y座標を固定する
    worldTransform_.translate.y = std::clamp(worldTransform_.translate.y, 0.0f, 2.4f);

}

bool Key::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), worldTransform_.translate);
}

void Key::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionChair) {
        if (!isChairHit_) {
            isChairHit_ = true;
            velocity_.y = 0.0f;
        }
    }
}

Vector3 Key::GetWorldPosition() const
{

    return worldTransform_.translate;
    //親子関係を付けている場合はこれ
//YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}
