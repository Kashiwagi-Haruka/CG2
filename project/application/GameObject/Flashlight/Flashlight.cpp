#include "Flashlight.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/Player/Player.h"

bool Flashlight::isSendGetLightMessage_ = false;

Flashlight::Flashlight()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/light", "light");
    obj_->SetModel("light");
    SetAABB({ .min = {-0.1f,-0.2f,-0.1f},.max = {0.1f,0.1f,0.1f} });
    SetCollisionAttribute(kCollisionItem);
    SetCollisionMask(kCollisionPlayer);
}

void Flashlight::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        isRotateY_ = true;
    }
}

Vector3 Flashlight::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Flashlight::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Flashlight::Update()
{
    if (isRotateY_) {
        transform_.rotate.y += YoshidaMath::kDeltaTime;
        obj_->SetRotate(transform_.rotate);
    }
    isRotateY_ = false;

    obj_->SetTransform(transform_);
    obj_->Update();
   
    spotLight_.position = obj_->GetTranslate();
    spotLight_.direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 10.0f;
   
}

void Flashlight::Initialize()
{

    isRotateY_ = false;
    isGetLight_ = false;
    isSendGetLightMessage_ = false;

    obj_->Initialize();
    transform_.translate = {8.0f,0.1f,1.0f};
    transform_.rotate = { 0.0f,0.0f,0.0f };
    transform_.scale = { 1.0f,1.0f,1.0f };
    SetLight();
}

void Flashlight::Draw()
{
 
    obj_->Draw();
}

void Flashlight::SetLight()
{
    spotLight_.color = { 1.0f, 1.0f, 0.5f, 1.0f };
    spotLight_.position = obj_->GetTranslate();
    spotLight_.direction = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    spotLight_.intensity = 10.0f;
    spotLight_.distance = 7.0f;
    spotLight_.decay = 2.0f;
    spotLight_.cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLight_.cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
}

void Flashlight::CheckCollision()
{

    if (PlayerCommand::GetInstance()->InteractTrigger()) {
        //keyとrayの当たり判定
        if (Player::GetIsGrab()) {
            isGetLight_ = false;
            Player::SetIsGrab(false);
        } else {
            if (OnCollisionRay()) {
                isGetLight_ = true;
                Player::SetIsGrab(true);
                isSendGetLightMessage_ = true;
            }

        }

    }

    if (isGetLight_) {
        // カーソルに追従させて持ち上げる処理
        Vector3 origin = playerCamera_->GetTransform().translate;
        transform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
        transform_.translate.y = (std::max)(transform_.translate.y, 0.0f);

    } 

    //y座標を固定する
    transform_.translate.y = std::clamp(transform_.translate.y, 0.0f, 2.4f);
}

bool Flashlight::OnCollisionRay()
{
     return playerCamera_->OnCollisionRay(GetAABB(), transform_.translate);
}
