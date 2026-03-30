#pragma once
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include<memory>
#include "Light/CommonLight/SpotCommonLight.h" 
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
class Camera;
class Player;
class Flashlight :
    public YoshidaMath::Collider
{
public:
    Flashlight();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    void SetCamera(Camera* camera);
    void Update();
    void Initialize();
    void Draw();
    void SetLight();
    SpotCommonLight& GetSpotLight() { return spotLight_; };
    void CheckCollision();
    void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    static bool IsGetLightMessage() { return isSendGetLightMessage_; }
    void UpdateSpotLight();
    void SetPlayer(Player* player) { player_ = player; }
    static bool IsGetLight() { return isGetLight_; }
private:
    bool OnCollisionRay();
private:
    Player* player_ = nullptr;
    PlayerCamera* playerCamera_ = nullptr;
    Transform transform_ = {};
    std::unique_ptr<Object3d>obj_ = nullptr;
    static bool isGetLight_;
  //SpotLight
    SpotCommonLight spotLight_;

    static bool isSendGetLightMessage_;
};

