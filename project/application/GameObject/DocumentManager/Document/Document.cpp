#include "Document.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include "Object3d/Object3dCommon.h"
#include"GameObject/SEManager/SEManager.h"
PlayerCamera* Document::playerCamera_ = nullptr;
bool Document::isRayHit_ = false;

Document::Document()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/document", "document");
    obj_->SetModel("document");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,0.3f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

    transform_.scale = { 10.0f,10.0f,10.0f };
    transform_.rotate = { 0.0f,0.0f,0.0f };
    transform_.translate = { 0.0f,0.0f,0.0f };
}

void Document::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 Document::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}

void Document::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Document::Update()
{

    CheckCollision();
    obj_->SetEnableLighting(false);
    obj_->SetTransform(transform_);
    obj_->Update();
}

void Document::Initialize()
{
    isRayHit_ = false;
    obj_->Initialize();
}

void Document::Draw()
{
    Object3dCommon::GetInstance()->DrawCommon();
    obj_->Draw();

}


void Document::CheckCollision()
{
    isRayHit_ = OnCollisionRay();
    if (isRayHit_) {
        //rayの当たり判定
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (!PlayerCommand::GetIsGrab()) {
                //書類を見れる
                SEManager::SoundPlay(SEManager::PAPER);
            }
        }
    }

}


bool Document::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
