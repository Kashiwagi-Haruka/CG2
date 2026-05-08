#pragma once
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Primitive/Primitive.h"
#include<memory>
class GitantGettlemanCollider : public YoshidaMath::Collider
{
public:
    GitantGettlemanCollider();
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;
    const Matrix4x4& GetWorldMatrix() const { return primitive_->GetWorldMatrix(); };
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void RegisterEditor(const std::string& name) { primitive_->RegisterEditor(name); };
    void Initialize(const YoshidaMath::ColliderType& type);

private:
    std::unique_ptr<Primitive>primitive_ = nullptr;
   
};

