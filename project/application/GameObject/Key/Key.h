#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/TimeCard/TimeCardWatch.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>

class Key
{
public:
    Key();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetModel(const std::string& filePath);
    AABB GetAABB();
    Transform& GetTransform() { return collisionTransform_; }
    void CheckCollision();
    bool OnCollisionRay();
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
#ifdef _DEBUG
    std::unique_ptr<Primitive>primitive_ = nullptr;
#endif
    AABB localAABB_ = {};
    Transform collisionTransform_ = {};
    PlayerCamera* playerCamera_ = nullptr;

	Transform worldTransform_ = {};
};

