#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"

#include<memory>

class Camera;

class Door :
    public YoshidaMath::Collider
{
public:
    Door();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    void Update();
    void Initialize();
    void Draw();
    void CheckCollision();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
  
private:
    void Animation();
    bool OnCollisionRay();
    const std::string animationGroupName_ = "sizuku";
    //骨
    std::unique_ptr<Skeleton> skeleton_{};
    //スキン
    SkinCluster skinCluster_{};
    bool animationFinished_ = false;
    std::string desiredAnimationName = "0Idle";
    const float kAnimationBlendDuration_ = 0.3f;
    Animation::AnimationData blendedPoseAnimation_{};
    PlayerCamera* playerCamera_ = nullptr;
    std::unique_ptr<Object3d>obj_ = nullptr;
};

