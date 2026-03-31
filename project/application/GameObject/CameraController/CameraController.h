#pragma once
#include"Vector3.h"
#include"GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "DebugCamera.h"

#include<memory>

class CameraController
{
public:
    static CameraController* GetInstance() {
        static CameraController instance;
        return &instance;
    }
    CameraController();
    void SetPlayer(Player* player) { playerCamera_->SetPlayer(player); }
    PlayerCamera* GetPlayerCamera() { return playerCamera_.get(); }
    void Initialize();
    void Update();
    Transform& GetTransform() {
        return eventCameraTransform_;
    }

    void SetEventMode(const bool enable) {
        isEventMode_ = enable;

    }void MoveTo(const Vector3& startPos, const Vector3& startRot,
        const Vector3& endPos, const Vector3& endRot, float time) {
        eventCameraTransform_.translate = startPos;
        eventCameraTransform_.rotate = startRot;
        targetPos_ = endPos;
        targetRot_ = endRot;
        lerpTime_ = time;
        currentLerp_ = 0.0f;
    };
    void MoveFromPlayerToTarget(const Vector3& endPos, const Vector3& endRot, float time) {
        SetPlayerCameraTransformToEventCameraTransform();
        targetPos_ = endPos;
        targetRot_ = endRot;
        lerpTime_ = time;
        currentLerp_ = 0.0f;
        isLerpEnd_ = false;
    };

    void MoveFromCurrentPosToPlayer(float time) {
        SetPlayerCameraTransformToTarget();
        lerpTime_ = time;
        currentLerp_ = 0.0f;
        isLerpEnd_ = false;
    }

    void MoveToPlayer(const Vector3& startPos, const Vector3& startRot, float time) {
        eventCameraTransform_.translate = startPos;
        eventCameraTransform_.rotate = startRot;
    
        SetPlayerCameraTransformToTarget();
        lerpTime_ = time;
        currentLerp_ = 0.0f;
        isLerpEnd_ = false;
    }

    void SetPlayerCameraTransformToTarget();
    void SetCameraTransform(const Vector3& pos, const Vector3& rot);
    //PlayerCameraの位置をセットする
    void SetPlayerCameraTransformToEventCameraTransform();
    bool IsLerpEnd() { return (isLerpEnd_ && currentLerp_ == 1.0f); }
private:
    void FollowPlayer();
private:
    Transform eventCameraTransform_;
    std::unique_ptr<PlayerCamera> playerCamera_ = nullptr;

    std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
    bool useDebugCamera_ = false;
    bool isLerpEnd_ = false;
    bool isEventMode_ = false;
    Vector3 targetPos_;
    Vector3 targetRot_;
    float lerpTime_ = 1.0f;
    float currentLerp_ = 0.0f;
};

