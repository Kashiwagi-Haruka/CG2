#pragma once
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Object3d/Object3d.h"
#include "PlayerParameters.h"
#include "RigidBody.h"
#include"Audio.h"
#include <memory>
#include <algorithm>

class Camera;
class Player : public YoshidaMath::Collider {
private:
    static constexpr const char* kParameterFileName = "playerParameters.json";
#pragma region // 体やメッシュの情報
    // 体のObj
    std::unique_ptr<Object3d> bodyObj_ = nullptr;
    // アニメーション
    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};
    const std::string animationGroupName_ = "Man";
    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Idle";
#pragma endregion
    // 体の座標
    Transform transform_{};
    // 速度
    Vector3 velocity_ = { 0.0f };
    Vector3 forward_ = { 0.0f };
    // 移動の速さ
    float moveSpeed_ = { 0.0f };
    AABB localAABB_ = { 0.0f };

    // プレイヤーのパラメータ
    PlayerParameters parameters_{};
    std::string parameterStatusMessage_{};
    float hp_ = 3.0f;
    static constexpr float kMaxHp_ = 3.0f;
    float damageCooldownTimer_ = 0.0f;


    void SaveParameters();
    void LoadParameters();


    static constexpr const char* kLeftFootJointName = "foot.L";
    static constexpr const char* kRightFootJointName = "foot.R";
    static constexpr float kFootContactHalfWidth = 0.08f;
    static constexpr float kFootContactHeight = 0.06f;
    static constexpr float kWalkFootstepInterval = 0.40f;
    static constexpr float kSneakFootstepInterval = 0.55f;

    float soundTimer_ = 0.0f;
    bool leftFootGrounded_ = false;
    bool rightFootGrounded_ = false;

    void ResetFootContactState();
    void UpdateFootContact(Collider* collider);
    bool CheckFootContact(Collider* collider, const char* jointName) const;
    bool IsMovingHorizontally() const;
    void PlayFootstepSE();
    // 移動
    void Move();
    // 重力処理
    void Gravity();
    // アニメーション
    void Animation();


public:
    SkinCluster& GetSkinCluster() { return skinCluster_; };
    Matrix4x4 GetJointMatrix(const char* jointName)const;
    Vector3 GetJointWorldPos(const char* jointName)const;
    void OnCollision(Collider* collider) override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    // 障害物との衝突処理
    void OnCollisionObstacle();
    Transform& GetTransform() { return transform_; };
    // 前方のベクトルを取得する
    const Vector3& GetForward() const { return forward_; };
    // ワールド行列の取得
   const Matrix4x4& GetWorldMatrix()const override {
       return bodyObj_->GetWorldMatrix();
   };
   void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
   void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    void SetTransform(const Transform& transform) { transform_ = transform; }
    // コンストラクタ
    Player();
    // カメラのセッター
    void SetCamera(Camera* camera);
    // 初期化
    void Initialize();
    // 更新処理
    void Update();
    // 描画処理
    void Draw();
    // デバック
    void Debug();
    const float GetDamageCoolDownTimer() { return damageCooldownTimer_; }
    const bool IsDie() { return hp_ <= 0.0f; };
    const float GetHP() { return hp_; };
    const float GetMaxHP() { return  kMaxHp_; }
    void UpdatePlayerDamage(const float deltaTime);
    void ApplyPlayerDamage(float damageAmount);
	void SetHP(float hp) { hp_ = std::clamp(hp, 0.0f, kMaxHp_); }
};