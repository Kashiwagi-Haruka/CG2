#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/TimeCard/TimeCardWatch.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>

class Key : public YoshidaMath::Collider
{
public:
    Key();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    void SetModel(const std::string& filePath);
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    static bool GetGetKeyMessage() { return isSendGetKeyMessage_; }
    static bool IsGetKey() { return isGetKey_; }
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
    PlayerCamera* playerCamera_ = nullptr;
    Transform worldTransform_ = {};
    Vector3 velocity_ = { 0.0f };
    bool isChairHit_ = false;
   static bool isGetKey_;
    static bool isSendGetKeyMessage_;
    bool isLockerHit_ = false;
};

