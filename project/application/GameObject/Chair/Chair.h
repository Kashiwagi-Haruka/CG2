#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include<memory>

class Camera;

class Chair :
    public YoshidaMath::Collider
{
public:
    Chair();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    void Update();
    void Initialize();
    void Draw();
  static  void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    void SetTransform(const Transform& transform) { transform_ = transform; };
    void SetMirrorTransform(Transform* transform) { mirrorTransform_ = transform; };
    Transform& GetTransform() { return transform_; };
private:
    bool OnCollisionRay();
   static PlayerCamera* playerCamera_;
    Transform transform_ = {};
    std::unique_ptr<Object3d>obj_ = nullptr;
    Transform* mirrorTransform_ = nullptr;
    Vector3 velocity_ = { 0.0f };
};

