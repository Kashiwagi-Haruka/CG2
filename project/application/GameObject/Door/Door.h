#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include"AutoLockSystem.h"
#include"Audio.h"

#include<memory>

class Camera;

class Door :
    public YoshidaMath::Collider
{
public:
    Door();
    ~Door();
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
    void SetIsGetKey(bool* flag) { isGetKey_ = flag; };
    std::unique_ptr<AutoLockSystem>& GetAutoLockSystem() { return autoLockSystem_; };
    void SetAnimationName(const std::string& name) { desiredAnimationName = name; }
    bool GetIsOpen() { return isOpen_; };

    static bool GetLockMassage() { return isSendLockMessage_; };
    static bool GetOpenMassage() { return isSendOpenMessage_; };
    static void SetLockMassage(const bool flag) {  isSendLockMessage_ = flag; };
    static void SetOpenMassage(const bool flag) {  isSendOpenMessage_ = flag; };
    void SetIsKeyPtr(bool* flagPtr) { isGetKey_ = flagPtr; }
private:
    static bool isSendLockMessage_;
    static bool isSendOpenMessage_;
    static SoundData doorLockSE_;
    static SoundData doorOpenSE_;
    bool* isGetKey_ = nullptr;
    bool isOpen_ = false;

    void Animation();
    bool OnCollisionRay();
    const std::string animationGroupName_ = "Door";
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
    Matrix4x4 worldMat_ = {};
    //オートロック
    std::unique_ptr<AutoLockSystem>autoLockSystem_ = nullptr;
};

