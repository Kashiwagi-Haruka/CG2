#pragma once
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include <Object3d/Object3d.h>
#include <memory>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>

// HintSheet と Document の共通の親クラス
class ReadableObject : public YoshidaMath::Collider {
protected:
    std::unique_ptr<Object3d> obj_ = nullptr;
    static PlayerCamera* playerCamera_;

    bool isRayHit_ = false;
    bool isLooking_ = false; // DocumentのisDocumentLook_もこれに統一
    Matrix4x4* parentMatrix_ = nullptr;
    bool* isOpenAnimationEndPtr_ = nullptr;
    // レイとの当たり判定共通処理
    virtual void CheckCollision();
public:

    // 子クラスで個別に実装させたいもの（純粋仮想関数）
    virtual void Update() = 0;
    virtual void DrawUI() = 0; // HintSheetはText、DocumentはSpriteを描画
    virtual ~ReadableObject() = default;
    // 共通の描画処理（アウトラインなど）
    virtual void Draw();
    /// @brief 衝突時コールバック関数
    virtual void OnCollision(Collider* collider)override;

    void SetOpenAnimationEndPtr(bool* flag) { isOpenAnimationEndPtr_ = flag; };
    void SetParentMatrix(Matrix4x4* parentMatrix);

    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;

    // ゲッター
    bool GetIsLooking() const { return isLooking_; }
    bool GetIsRayHit() const { return isRayHit_; }
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); }
    void SetCamera(Camera* camera)
    {
        obj_->SetCamera(camera);
        obj_->UpdateCameraMatrices();
    }

    Matrix4x4* GetParentMatrix() const { return parentMatrix_; } // 追加: 親マトリックスを取得
};