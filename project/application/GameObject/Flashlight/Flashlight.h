#pragma once
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include<memory>
#include "Light/SpotLight.h" 
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
class Camera;

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
    SpotLight& GetSpotLight() { return spotLight_; };
    void CheckCollision();
    void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    static bool IsGetLightMessage() { return isSendGetLightMessage_; }
    void UpdateSpotLight();
private:
    bool OnCollisionRay();
private:
    PlayerCamera* playerCamera_ = nullptr;
    Transform transform_ = {};
    std::unique_ptr<Object3d>obj_ = nullptr;
    //SpotLight
    SpotLight spotLight_;
    bool isGetLight_ = false;
    static bool isSendGetLightMessage_;
};

