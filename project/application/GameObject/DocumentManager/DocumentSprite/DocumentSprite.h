#pragma once
#include "Sprite.h"
#include<array>
#include <memory>

class DocumentSprite
{

public:
    enum Documents {
        DOC_TIMECARD_WATCH,
        DOC_SECRET,
        DOC_MAX
    };

    DocumentSprite();
    void Initialize();
    void Update();
    void Draw();
    void SetTexture(const Documents num);
private:
    Vector2 position_ = { 0.0f };
    
    std::unique_ptr<Sprite> sprite_ = nullptr;
    std::array<uint32_t, DOC_MAX>handle_;

    // --- ここからスクロールバー用に追加 ---
    std::unique_ptr<Sprite> scrollbarTrackSprite_ = nullptr; // バーの背景
    std::unique_ptr<Sprite> scrollbarThumbSprite_ = nullptr; // バーのつまみ
    uint32_t scrollbarTexHandle_ = 0; // スクロールバー用のテクスチャ
};

