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

class GiantGentleMan : public YoshidaMath::Collider
{
public:
    GiantGentleMan();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); }

    void Initialize();
    void Update();
    void Draw();

    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void SetCamera(Camera* camera);

    static bool IsRayHit() { return isRayHit_; }
    //面が向き合ってるか
    bool IsFacingSurface(const Matrix4x4& cameraMat);
private:
    void CheckCollision();
    bool OnCollisionRay();
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
    std::string animationGroupName_ = "GiantGentleman";
    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    const std::string desiredAnimationName = "SleepStand";
    static bool isRayHit_;
    //ポータルが出来る範囲
    const float kPortalCreatableAngleRange_ = 0.5f;
};

