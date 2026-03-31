#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"

class Desk : public YoshidaMath::Collider
{
public:
    Desk();
    void Initialize();
    void Update();
    void Draw();
    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void SetCamera(Camera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    void SetAnimationGroupName(const std::string& name){ animationGroupName_ = name; }
    bool IsRayHit() { return isRayHit_; }
private:
    // アニメーション
    void Animation();
private:
    static PlayerCamera* playerCamera_;
    std::unique_ptr<Object3d>obj_ = nullptr;
    AABB localAABB_;
    // アニメーション
    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};
     std::string animationGroupName_ = "Desk";
    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Idle";
    bool isRayHit_ = false;
    
};

