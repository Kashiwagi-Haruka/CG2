#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Primitive/Primitive.h"
#include<memory>

class Camera;

class Wall :
    public YoshidaMath::Collider
{
public:
    Wall();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    void Update();
    void Initialize();
    void Draw();
    void SetCamera(Camera* camera);
    void AdjustAABB();
private:
    std::unique_ptr<Primitive>primitive_ = nullptr;
};

