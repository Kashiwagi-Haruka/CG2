#pragma once
#include <Transform.h>
#include <string>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include "Text/Text.h" // Creditクラスで使用しているTextクラス
#include <GameObject/ReadableObject/ReadableObject.h>
class HintSheet : public ReadableObject
{
public:
    HintSheet();
    void Update()override;
    /// @brief 2Dテキストの描画 (UI描画フェーズで呼ぶ)
    void DrawUI()override;

    /// @brief 初期化
    /// @param fontHandle Text描画用のフォントハンドル
    /// @param textFilePath 読み込むテキストファイルのパス (例: "Resources/TXT/Hint.txt")
    void Initialize(uint32_t fontHandle, const std::string& textFilePath);


    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider) override;
   
private:
    void CheckCollision()override;
private:


    Text hintText_; // ヒント表示用のTextオブジェクト
};