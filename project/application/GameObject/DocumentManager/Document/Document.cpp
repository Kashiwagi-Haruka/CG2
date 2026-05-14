#include "Document.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"

#include"GameObject/SEManager/SEManager.h"

namespace {
    const Vector4 kRayHitOutlineColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    const float kRayHitOutlineWidth = 10.0f;
} // namespace
Document::Document()
{
    obj_ = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/document", "document");
    obj_->SetModel("document");
    SetAABB({ .min = {-0.0625f,0.0f,-0.0625f},.max = {0.0625f,0.3f,0.0625f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

}

void Document::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

void Document::DrawUI()
{
}

void Document::Update()
{
    CheckCollision();
    obj_->SetEnableLighting(false);

    Transform transform = obj_->GetTransform();

    if (parentMatrix_) {
        //親がセットされていればペアレントする
        Matrix4x4 child = Function::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
        obj_->SetWorldMatrix(Function::Multiply(child, *parentMatrix_));
    } else {
        obj_->SetTransform(transform);
    }

    obj_->Update();

    worldPos_ = GetWorldPosition();
}

void Document::Initialize(const std::string name)
{
    isRayHit_ = false;
    isLooking_ = false;
    obj_->Initialize();
    obj_->RegisterEditor(name);
    obj_->SetOutlineColor(kRayHitOutlineColor);
    obj_->SetOutlineWidth(kRayHitOutlineWidth);
}


//void Document::CheckCollision()
//{
//    isRayHit_ = OnCollisionRay();
//
//    // トリガーが押された時の処理
//    if (PlayerCommand::GetInstance()->InteractTrigger()) {
//
//        // 物を持っていない状態を前提とする場合
//        if () {
//            if (isRayHit_) {
//                isLooking_ = !isLooking_;
//                SEManager::SoundPlay(SEManager::PAPER);
//            }
//        }
//    }
//
//    if (!isRayHit_) {
//        //外れたら強制的に終了する
//        isLooking_ = false;
//    }
//
//}
