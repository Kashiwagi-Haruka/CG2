#pragma once
#include <vector>
#include <memory>
#include "HintSheet.h"
#include "Sprite.h"

class HintSheetManager {
public:
    HintSheetManager() = default;
    ~HintSheetManager() = default;

    /// @brief 初期化
    void Initialize();

    /// @brief ヒントシートを追加する
    /// @param transform 配置場所
    /// @param textFilePath 読み込むテキストファイルパス
    void AddHintSheet( const std::string& textFilePath);

    void Update();

    /// @brief 3Dモデルの描画
    void Draw();

    /// @brief テキストUIの描画
    void DrawUI();

    /// @brief 全ヒントシートにカメラをセット
    void SetCamera(Camera* camera);

    /// @brief プレイヤーカメラをセット
    void SetPlayerCamera(PlayerCamera* playerCamera);

    /// @brief 全ヒントシートのリストを取得
    std::vector<std::unique_ptr<HintSheet>>& GetHintSheets() { return hintSheets_; }
    void SetParentMatrix(const uint32_t index,Matrix4x4* parentMatrix);
private:
    std::unique_ptr<Sprite>sprite_ = nullptr;
    std::vector<std::unique_ptr<HintSheet>> hintSheets_;
    uint32_t fontHandle_ = 0;
    Camera* camera_ = nullptr;
};