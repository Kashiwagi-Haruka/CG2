#pragma once
#include <Transform.h>
#include <string>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include "Text/Text.h" // Creditクラスで使用しているTextクラス

class HintSheet : public YoshidaMath::Collider
{
public:
    HintSheet();

    /// @brief 初期化
    /// @param fontHandle Text描画用のフォントハンドル
    /// @param textFilePath 読み込むテキストファイルのパス (例: "Resources/TXT/Hint.txt")
    void Initialize(uint32_t fontHandle, const std::string& textFilePath);
    void SetParentMatrix(Matrix4x4* parentMatrix);
    void Update();

    /// @brief 3Dモデルの描画
    void Draw();

    /// @brief 2Dテキストの描画 (UI描画フェーズで呼ぶ)
    void DrawUI();

    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    void SetCamera(Camera* camera);

    // レイの当たり判定用
    void SetIsRayHit(bool isHit) { isRayHit_ = isHit; }

    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider) override;

    /// @brief ワールド座標を取得する
    Vector3 GetWorldPosition() const override;
    const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };

    bool GetIsLooking() const { return isLooking_; }

private:
    void InteractUpdate();

private:
    bool isLooking_ = false;       // ヒントを見ている状態か
    bool isRayHit_ = false;        // プレイヤーの視線(レイ)が当たっているか
    static PlayerCamera* playerCamera_;
    Matrix4x4* parentMatrix_ = nullptr;
    std::unique_ptr<Object3d> obj_ = nullptr;

    Text hintText_; // ヒント表示用のTextオブジェクト
};