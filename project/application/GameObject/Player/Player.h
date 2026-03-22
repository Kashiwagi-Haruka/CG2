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
#include <string>

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


    //何かをつかんでいるかどうか
    static bool isGrab_;
	// プレイヤーのパラメータ
	PlayerParameters parameters_{};
	std::string parameterStatusMessage_{};

	void SaveParameters();
	void LoadParameters();

	static constexpr const char* kLeftFootJointName = "foot.L";
	static constexpr const char* kRightFootJointName = "foot.R";
	static constexpr float kFootContactHalfWidth = 0.08f;
	static constexpr float kFootContactHeight = 0.06f;
	static constexpr float kWalkFootstepInterval = 0.40f;
	static constexpr float kSneakFootstepInterval = 0.55f;

	SoundData footStepSE;
	float soundTimer_ = 0.0f;
	bool leftFootGrounded_ = false;
	bool rightFootGrounded_ = false;

	void ResetFootContactState();
	void UpdateFootContact(Collider* collider);
	bool CheckFootContact(Collider* collider, const char* jointName) const;
	bool IsMovingHorizontally() const;
	void PlayFootstepSE();


public:
    static bool GetIsGrab() { return isGrab_; };
    static void SetIsGrab(bool flag) { isGrab_ = flag; }
    void OnCollision(Collider* collider) override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    // 障害物との衝突処理
    void OnCollisionObstacle();
    Transform& GetTransform() { return transform_; };
    // 前方のベクトルを取得する
    const Vector3& GetForward() const { return forward_; };
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
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
    // 移動
    void Move();
    // 重力処理
    void Gravity();

    // アニメーション
    void Animation();
    // ワールド行列の取得
    const Matrix4x4& GetWorldMatrix() const { return bodyObj_->GetWorldMatrix(); }
};