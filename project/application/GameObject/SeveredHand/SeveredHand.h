#pragma once
#include <Camera.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/ReadableObject/ReadableObject.h>

class SeveredHand : public ReadableObject
{
public:
    SeveredHand();
    void Initialize();
    void Update()override;
    void SetPlayerCamera(PlayerCamera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標SetParentMatrix
    Vector3 GetWorldPosition() const override; 
    void DrawUI() {};
private:
    Vector3 worldPos_ = { 0.0f };
};

