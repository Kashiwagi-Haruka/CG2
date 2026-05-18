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
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"

class Locker : public YoshidaMath::Collider {
public:
    Locker();
    void Initialize();
    void Update();
    void Draw();
    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
  	void SetEditorRegistrationName(const std::string& name) { editorRegistrationName_ = name; }
    void SetCamera(Camera* camera);
  
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    bool IsRayHit() { return isRayHit_; }
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };
    void SetAnimationGroupName(const std::string& name) { animationGroupName_ = name; }
    bool GetIsOpen() { return isOpen_; }
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>& GetColliders() {
        return colliders_ ;
    }
    const Vector3& GetRotate() { return obj_->GetRotate(); }
    bool GetIsPlayerIn() { return isPlayerIn_; };
    void SetIsPlayerIn(bool flag) { isPlayerIn_ = flag;  }
    // ★追加：フロントコライダーのワールド座標（出口）を取得する
    Vector3 GetForward() {
        return YoshidaMath::GetForward(obj_->GetWorldMatrix());
    }
private:
    void Animation();
    void CheckCollision();
private:
    bool isOpen_ = false;
    bool isRayHit_ = false;
    bool isPlayerIn_ = false;
    bool isPlayerPreIn_ = false;
    static PlayerCamera* playerCamera_;
    std::unique_ptr<Object3d>obj_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>colliders_;

    // アニメーション
    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};
    std::string animationGroupName_ = "Locker";
    const float kAnimationBlendDuration_ = 0.5f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Idle";
	std::string editorRegistrationName_ = "Locker";
    Matrix4x4 worldMatrix_;
};
