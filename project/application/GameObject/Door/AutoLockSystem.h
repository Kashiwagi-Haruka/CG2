#pragma once
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Primitive/Primitive.h"

class AutoLockSystem :public YoshidaMath::Collider
{
public:
    AutoLockSystem();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); }
    void Update();
    void Initialize();
    void Draw();
    void SetCamera(Camera* camera);
    void SetParentMat(Matrix4x4* parentMat) { parentMat_ = parentMat; }
    bool IsPlayerHit() { return isPlayerHit_; };
    bool IsPlayerPreHit() { return isPlayerPreHit_; };
    void SetTranslate(const Vector3& translate) { translate_ = translate; }
private:
    std::unique_ptr<Primitive>obj_ = nullptr;
    Matrix4x4* parentMat_ = nullptr;
    bool isPlayerHit_ = false;
    bool isPlayerPreHit_ = false;
    Vector3 translate_ = { 0.0f };
};


