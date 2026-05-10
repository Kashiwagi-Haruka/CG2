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
#include"GiantGentlemanHead.h"
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"
#include<unordered_map>
class GiantGentleMan 
{
public:

public:

    GiantGentleMan();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerPos(Vector3* playerPos);
    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void SetCamera(Camera* camera);
    bool CanMakePortal() { return canMakePortal_; }
    bool IsMakePortal() { return isMakePortal_; }
    //面が向き合ってるか
    bool IsFacingSurface(const Matrix4x4& cameraMat);
    GiantGentlemanHead* GetGiantGentlemanHead() { return head_.get(); }
    std::unordered_map<std::string,std::unique_ptr<ObjectCollider>>& GetGiantGentlemanHand() { return colliders_; }
    Transform& GetCollisionTransform() { return collisionTransform_; };
private:
    void CheckCollision();
    bool OnCollisionRay();
    // アニメーション
    void Animation();

private:
    Transform collisionTransform_ = { 0.0f };
    Vector3* playerPos_ = nullptr;
    std::unique_ptr<GiantGentlemanHead>head_ = nullptr;
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>colliders_;

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
    bool isRayHit_;
    bool canMakePortal_ = false;
    bool isMakePortal_ = false;
    //ポータルが出来る範囲
    const float kPortalCreatableAngleRange_ = 0.5f;
};

