#pragma once
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Primitive/Primitive.h"
#include<memory>
#include<functional>

class ObjectCollider : public YoshidaMath::Collider
{
public:
    ObjectCollider();
    virtual void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;
    const Matrix4x4& GetWorldMatrix() const { return primitive_->GetWorldMatrix(); };
    void Update();
    void Draw();
    void SetTransform(const Transform& transform) { primitive_->SetTransform(transform); }
    void SetCamera(Camera* camera);
    void RegisterEditor(const std::string& name) { primitive_->RegisterEditor(name); };
    void Initialize(const YoshidaMath::ColliderType& type);
    void SetParentMatrix(Matrix4x4* parentMatrix) { parent_ = parentMatrix; }
    void SetTextureIndex(const uint32_t textureIndex) { primitive_->SetTextureIndex(textureIndex); }
    void SetParentScale(Vector3* scale) { scale_ = scale; }
protected:
    std::unique_ptr<Primitive>primitive_ = nullptr;
    Matrix4x4* parent_ = nullptr;
    Vector3* scale_ = nullptr;
};

