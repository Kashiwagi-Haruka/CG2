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
#include"GameSave/GameSave.h"

class Gentleman : public YoshidaMath::Collider
{
public:
    Gentleman();
    void Initialize();
    void Update();
    void Draw();
    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    static void SetPlayerTransform(Transform* transform) { playerTransform_ = transform; };
    static void SetProgressSaveData(ProgressSaveData* progressSaveData) { progressSaveData_ = progressSaveData; }
    void SetCamera(Camera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    bool IsRayHit() { return isRayHit_; }
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); }
    void SetAnimationName(const std::string& name) { desiredAnimationName = name; };
private:
    void SwichCommand();
    // アニメーション
    void Animation();
    //セーブ
    void Save(const int slotIndex = 0);
private:
    static PlayerCamera* playerCamera_;
    static Transform* playerTransform_;
    static ProgressSaveData* progressSaveData_;
    std::unique_ptr<Object3d>obj_ = nullptr;
    AABB localAABB_;
    // アニメーション
    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};
    std::string animationGroupName_ = "Gentleman";
    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Idle";
    bool isRayHit_ = false;
    bool isPreOnCollisionRay_ = false;
    uint32_t animationNum = 0;
};

