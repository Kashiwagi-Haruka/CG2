#include "ReadableObject.h"
#include "Object3d/Object3dCommon.h"
#include"GameObject/KeyBindConfig.h"
#include"GameObject/SEManager/SEManager.h"

PlayerCamera* ReadableObject::playerCamera_ = nullptr;

void ReadableObject::SetParentMatrix(Matrix4x4* parentMatrix)
{
    assert(parentMatrix);
    parentMatrix_ = parentMatrix;
}

void ReadableObject::CheckCollision()
{
    isRayHit_ = playerCamera_->OnCollisionRay(GetAABB(), YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix()));
    // トリガーが押された時の処理  物を持っていない状態を前提とする場合
    if (PlayerCommand::GetInstance()->InteractTrigger() && !PlayerCommand::GetIsGrab()) {

        if (isRayHit_) {
            //開いたアニメーションが終わった時やポインタがないとき
            if (isOpenAnimationEndPtr_ && *isOpenAnimationEndPtr_ || !isOpenAnimationEndPtr_) {

                SEManager::SoundPlay(SEManager::PAPER);

                if (isLooking_) {
                    // すでにヒントを見ているなら「閉じる」
                    isLooking_ = false;
                    PlayerCommand::SetIsLook(false);

                } else {
                    //何も見ていないとき
                    if (!PlayerCommand::GetIsLook()) {
                        // ヒントを見ていない ＆ レイが当たっているなら「見る」
                        isLooking_ = true;

                        PlayerCommand::SetIsLook(true);
                    }
                }
            }


        }
    }

    if (!isRayHit_) {
        //外れたら強制的に終了する
        isLooking_ = false;
        PlayerCommand::SetIsLook(false);
    }
}

void ReadableObject::Draw()
{
    if (isRayHit_ && !isLooking_) {
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

void ReadableObject::OnCollision(Collider* collider)
{
}

Vector3 ReadableObject::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}
