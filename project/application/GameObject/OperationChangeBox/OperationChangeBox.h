#pragma once
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include "Transform.h"
#include <memory>

class Camera;
class PlayerCamera;
class Primitive;

class OperationChangeBox {
public:
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera) { playerCamera_ = playerCamera; }

    bool ConsumeInteractRequest();
    bool IsRayHit() const { return isRayHit_; }
    const Transform& GetTransform() const { return transform_; }
    Vector3 GetForward() const;

private:
    AABB GetAABB() const;
    bool OnCollisionRay() const;

private:
    std::unique_ptr<Primitive> box_ = nullptr;
    PlayerCamera* playerCamera_ = nullptr;
    Transform transform_{};
    bool isRayHit_ = false;
    bool interactRequested_ = false;
};
