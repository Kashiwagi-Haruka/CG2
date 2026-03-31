#pragma once
#include"Vector3.h"
#include"GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "DebugCamera.h"

#include<memory>

class CameraController
{
public:
    CameraController();
    void Initialize();
    void SetEventMode(const bool enable) {
        isEventMode_ = enable;

    }void MoveTo(const Vector3& startPos, const Vector3& startRot,
        const Vector3& endPos, const Vector3& endRot, float time) {
        eventCameraTransform_.translate = startPos;
        eventCameraTransform_.rotate = startRot;
        targetPos_ = endPos;
        targetRot_ = endPos;
        lerpTime_ = time;
        currentLerp_ = 0.0f;
    };

    void SetPlayer(Player* player) { playerCamera_->SetPlayer(player); }
    void Update();
    //PlayerCameraの位置をセットする
    void AppendPlayerCameraTransformToEventCameraTransform();
    void SetCameraTransform(const Vector3& pos, const Vector3& rot);
    PlayerCamera* GetPlayerCamera() { return playerCamera_.get(); }
private:
    void FollowPlayer();
private:
    Transform eventCameraTransform_;
    std::unique_ptr<PlayerCamera> playerCamera_ = nullptr;

    std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
    bool useDebugCamera_ = false;

    bool isEventMode_ = false;
    Vector3 targetPos_;
    Vector3 targetRot_;
    float lerpTime_ = 1.0f;
    float currentLerp_ = 0.0f;
};

