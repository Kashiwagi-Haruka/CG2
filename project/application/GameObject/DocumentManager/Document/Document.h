#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>

class Document : public YoshidaMath::Collider
{
public:
    Document();
    void Initialize(const std::string name);
    void Update();
    void Draw();
    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void SetCamera(Camera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    static bool IsRayHit() { return isRayHit_; }
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };
    bool GetDocumentLook() { return isDocumentLook_; }
    Vector3* GetTranslate() { return &pos_; };
private:
    bool isDocumentLook_ = false;
    static  bool isRayHit_;
    static PlayerCamera* playerCamera_;
    std::unique_ptr<Object3d>obj_ = nullptr;
    Vector3 pos_;
};

