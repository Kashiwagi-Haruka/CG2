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

class EdamameModel : public YoshidaMath::Collider
{
public:
    EdamameModel();
    void Initialize();
    void Update();
    void Draw();

    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    //トランスフォームの設定
    void SetTranslate(const Vector3& translate) { translate_ = translate; };
    //始まる
    void SetIsStartMove(const bool flag) { isStart_ = flag; }
    //落下開始
    void SetIsDropStart(const bool flag) { isDropStart_ = flag; }
    //カメラの設定
    void SetCamera(Camera* camera);
	// Rayヒット状態
	void SetRayHit(bool isRayHit) { isRayHit_ = isRayHit; }
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };
private:
    // アニメーション
    void Animation();
private:
    //始まる
    bool isStart_ = false;
    //落下開始
    bool isDropStart_ = false;
    bool isDrop_ = false;
    float speedY_ = 0.0f;
    float scaleTimer_ = 0.0f;
    Vector3 translate_ = { 0.0f };
    Vector3 rotate_ = { 0.0f };
    std::unique_ptr<Object3d>obj_ = nullptr;
    // アニメーション
    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};
	bool isRayHit_ = false;
    const std::string animationGroupName_ = "Edamame";
    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Speak";
};

