#pragma once
#include "WhiteBoard.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"

class WalkWhiteBoard :
    public WhiteBoard
{
private:
    //アニメーションクリップ
    static std::vector<Animation::AnimationData> animationClips_;
    //骨
    std::unique_ptr<Skeleton> skeleton_{};
    //スキン
    SkinCluster skinCluster_{};
    Animation::AnimationData blendedPoseAnimation_{};
    const std::string animationGroupName_ = "walkWhiteBoard";
    const float kAnimationBlendDuration_ = 0.3f;
    bool animationFinished_ = false;
    std::string desiredAnimationName = "Idle";
public:
    WalkWhiteBoard();
    void OnCollision(Collider* collider)override;
    void SetTargetPosPtr(Vector3* pos) { targetPos_ = pos; };
    static void LoadAnimation(const std::string& directionalPath, const std::string& filePath);
    void Initialize()override;
    void Update()override;
    void Draw()override;
    void ResetCollisionAttribute()override;
    void SetIsMove(const bool isMove) { isMove_ = isMove; }
    void SetCollisionAttributeNoneAndInitialize()override;

private:
    bool isMove_ = false;
  
    Vector3 velocity_ = { 0.0f };
    Transform transform_ = {};
    Vector3* targetPos_ = nullptr;
    void Animation();
};

