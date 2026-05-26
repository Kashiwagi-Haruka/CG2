#include "AutoLockSystem.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"cassert"

AutoLockSystem::AutoLockSystem()
{
    SetAABB({ .min = {-1.0f,0.0f,-0.375f},.max = {1.0f,0.02f,0.375f} });
    SetCollisionAttribute(kCollisionMat);
    SetCollisionMask(kCollisionPlayer);
}

void AutoLockSystem::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {
        //プレイヤーと当たった時
        isPlayerHit_ = true;
    }
}

void AutoLockSystem::Update()
{
    //前回のフレームを記録
    isPlayerPreHit_ = isPlayerHit_;
    isPlayerHit_ = false;

    ObjectCollider::Update();

}

void AutoLockSystem::Initialize()
{
    isPlayerHit_ = false;
    isPlayerPreHit_ = false;

    SetCollisionAttribute(kCollisionMat);
    SetCollisionMask(kCollisionPlayer);

    //BOXを入れる
    primitive_->Initialize(Primitive::Box, "Resources/TD3_3102/2d/floor.png");
}
