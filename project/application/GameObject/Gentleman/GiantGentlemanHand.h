#pragma once
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Primitive/Primitive.h"
#include<memory>
class GiantGentlemanHand : public YoshidaMath::Collider
{
public:
    GiantGentlemanHand();
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;
    const Matrix4x4& GetWorldMatrix() const { return primitive_->GetWorldMatrix(); };
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void RegisterEditor(const std::string& name) { primitive_->RegisterEditor(name); };
private:
    std::unique_ptr<Primitive>primitive_ = nullptr;
};

