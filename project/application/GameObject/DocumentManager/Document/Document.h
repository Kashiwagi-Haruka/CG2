#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include <GameObject/ReadableObject/ReadableObject.h>

class Document :public ReadableObject
{
public:
    Document();
    void Initialize(const std::string name);
    void Update()override;
    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void OnCollision(Collider* collider)override;
    Vector3* GetTranslate() { return &worldPos_; };
  void DrawUI()override;
private:
    Vector3 worldPos_;
};

