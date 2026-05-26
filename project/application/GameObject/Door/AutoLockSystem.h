#pragma once
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"

class AutoLockSystem :public ObjectCollider
{
public:
    AutoLockSystem();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    void Update();
    void Initialize();
    bool IsPlayerHit() { return isPlayerHit_; };
    bool IsPlayerPreHit() { return isPlayerPreHit_; };
    bool IsTrigger() { return !isPlayerHit_ && isPlayerPreHit_; }
private:
    bool isPlayerHit_ = false;
    bool isPlayerPreHit_ = false;
};


