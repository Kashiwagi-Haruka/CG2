#pragma once
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"
#include<unordered_map>
#include <Camera.h>

class GiantEnemy
{
public:
    //これを呼び出す前にModelをLoadする
    GiantEnemy();
    void Initialize();
    void SetName(const std::string name);
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    //面が向き合ってるか
    bool IsFacingSurface(const Matrix4x4& cameraMat);
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>& GetColliders() { return colliders_; }
    void OnCollisionWithFloor(YoshidaMath::Collider* collider);
    Transform& GetCollisionTransform() { return collisionTransform_; };
    bool IsWall() { return collisionAttribute_ == kCollisionWall; }
    Object3d* GetObject3d() const { return obj_.get(); }
    void ResetCollisionAttribute();
    //衝突マスクをセットするのとイニシャライズ
   void SetCollisionAttributeWallAndInitialize();
   bool OnCollisionRay(PlayerCamera* playerCamera);
   /// @brief 衝突属性を取得する
/// @return 衝突属性
   uint32_t GetCollisionAttribute() const { return collisionAttribute_; }
   /// @brief 衝突属性を設定する
/// @param attribute 衝突属性
   void SetCollisionAttribute(uint32_t attribute) { collisionAttribute_ = attribute; }
   //ターゲットを設定
   static void SetTargetPos(Vector3* targetPos) { targetPos_ = targetPos; }
private:
    // アニメーション
    void Animation();
    const Vector3 GetJointWordPos(const std::string name);
    const Matrix4x4 GetJointWorldMatrix(const std::string name);
    void ResetFootContactState();
private:
    std::unordered_map<std::string, Matrix4x4>parentMats_;
    Transform collisionTransform_ = { 0.0f };
    uint32_t collisionAttribute_ = kCollisionEnemy;	// 衝突属性

    std::unique_ptr<Object3d>obj_ = nullptr;
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>colliders_;
  
    bool leftFootGrounded_ = false;
    bool rightFootGrounded_ = false;
    float soundTimer_ = 0.0f;

    //=======================================
    //  アニメーション
    //=======================================

    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};
    std::string animationGroupName_ = "GiantEnemy";

    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Idle";
    //ポータルが出来る範囲
    const float kPortalCreatableAngleRange_ = 0.75f;
    static Vector3* targetPos_;
    Vector3 velocity_ = { 0.0f };
};

