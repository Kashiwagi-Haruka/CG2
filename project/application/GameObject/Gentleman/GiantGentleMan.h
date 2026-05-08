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
#include"GiantGentlemanHand.h"
#include<unordered_map>
class GiantGentleMan 
{
public:

public:

    GiantGentleMan();
    void Initialize();
    void Update();
    void Draw();

    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void SetCamera(Camera* camera);

    static bool IsRayHit() { return isRayHit_; }
    //面が向き合ってるか
    bool IsFacingSurface(const Matrix4x4& cameraMat);
    GiantGentlemanHead* GetGiantGentlemanHead() { return head_.get(); }
    std::unordered_map<std::string,std::unique_ptr<GiantGentlemanHand>>& GetGiantGentlemanHand() { return hands_; }

private:
    void CheckCollision();
    bool OnCollisionRay();
    // アニメーション
    void Animation();
private:



    std::unique_ptr<GiantGentlemanHead>head_ = nullptr;
    std::unordered_map<std::string, std::unique_ptr<GiantGentlemanHand>>hands_;

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

