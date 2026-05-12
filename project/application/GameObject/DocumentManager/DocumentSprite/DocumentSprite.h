#pragma once
#include "Sprite.h"
#include<array>
#include <memory>

class DocumentSprite
{

public:
    DocumentSprite();
    void Initialize(const std::string name);
    void Update();
    void Draw();
private:
    Vector2 position_ = { 0.0f };
    
    std::unique_ptr<Sprite> sprite_ = nullptr;
    // --- ここからスクロールバー用に追加 ---
    std::unique_ptr<Sprite> scrollbarTrackSprite_ = nullptr; // バーの背景
    std::unique_ptr<Sprite> scrollbarThumbSprite_ = nullptr; // バーのつまみ
    uint32_t scrollbarTexHandle_ = 0; // スクロールバー用のテクスチャ
};

