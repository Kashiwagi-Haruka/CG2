#include "Document.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include "Object3d/Object3dCommon.h"
#include"GameObject/SEManager/SEManager.h"
PlayerCamera* Document::playerCamera_ = nullptr;
bool Document::isRayHit_ = false;
namespace {
    const Vector4 kRayHitOutlineColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    const float kRayHitOutlineWidth = 10.0f;
} // namespace
Document::Document()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/document", "document");
    obj_->SetModel("document");
    SetAABB({ .min = {-0.5f,0.0f,-0.5f},.max = {0.5f,0.3f,0.5f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

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
    obj_->Update();
    pos_ = obj_->GetTranslate();
}

void Document::Initialize(const std::string name)
{
    isRayHit_ = false;
    isDocumentLook_ = false;
    obj_->Initialize();
    obj_->RegisterEditor(name);
    obj_->SetOutlineColor(kRayHitOutlineColor);
    obj_->SetOutlineWidth(kRayHitOutlineWidth);
}

void Document::Draw()
{
 
    if (isRayHit_&&!isDocumentLook_) {
        Object3dCommon::GetInstance()->DrawCommon();
        obj_->Draw();
        Object3dCommon::GetInstance()->DrawCommonOutline();
        obj_->Draw();
        Object3dCommon::GetInstance()->EndOutlineDraw();
    } else {
        Object3dCommon::GetInstance()->DrawCommon();
        obj_->Draw();
    }

}


void Document::CheckCollision()
{
    isRayHit_ = OnCollisionRay();

    // トリガーが押された時の処理
    if (PlayerCommand::GetInstance()->InteractTrigger()) {

        // 物を持っていない状態を前提とする場合
        if (!PlayerCommand::GetIsGrab()) {
            if (isRayHit_) {
                isDocumentLook_ = !isDocumentLook_;
                SEManager::SoundPlay(SEManager::PAPER);
            }
        }
    }

    if (!isRayHit_) {
        //外れたら強制的に終了する
        isDocumentLook_ = false;
    }

}

bool Document::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(GetAABB(), GetWorldPosition());
}
