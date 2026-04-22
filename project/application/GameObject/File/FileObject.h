#pragma once
#include <Transform.h>
#include <Camera.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>

class FileObject : public YoshidaMath::Collider
{
public:
    FileObject();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };
    void SetTransform(const Transform& transform) { transform_ = transform; }
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
    Transform transform_;
};

