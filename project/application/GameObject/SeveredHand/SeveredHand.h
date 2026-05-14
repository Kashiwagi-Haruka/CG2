#pragma once
#include <Camera.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>

class SeveredHand : public YoshidaMath::Collider
{
public:
    SeveredHand();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標SetParentMatrix
    Vector3 GetWorldPosition() const override;
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };
    void SetParentMatrix(Matrix4x4* parentMat) { parentMat_ = parentMat; }
private:
    Matrix4x4* parentMat_ = nullptr;
    bool isRayHit_ = false;
    bool isLook_ =  false;
    std::unique_ptr<Object3d>obj_ = nullptr;
    PlayerCamera* playerCamera_ = nullptr;
    Vector3 worldPos_ = { 0.0f };
};

