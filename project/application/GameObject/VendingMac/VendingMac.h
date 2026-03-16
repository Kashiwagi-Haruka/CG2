#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include<memory>
#include "Light/AreaLight.h"
#include"Audio.h"

class Camera;

class VendingMac :
    public YoshidaMath::Collider
{
public:
    VendingMac();
    ~VendingMac();
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
    AreaLight& GetAreaLight() { return  areaLight_; }
private:
    float GetVol(float length, float maxVol);
    static SoundData noise_;
    bool OnCollisionRay();
    PlayerCamera* playerCamera_ = nullptr;
    std::unique_ptr<Object3d>obj_ = nullptr;
    AreaLight areaLight_;
    Vector3 translate_ = { 0.0f };
};

