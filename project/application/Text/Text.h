#pragma once
#include <string>
#include <vector>
#include "Vector2.h"
#include "Vector4.h"
#include "Text/FreetypeManager/FreeTypeManager.h"
#include"Text/FreetypeManager/Font/Font.h"
#include"BlendMode/BlendModeManager.h"
#include"Audio.h"
enum class TextAlign {
    Left,
    Center,
    Right
};
enum class VerticalAlign { 
    Top,
    Middle,
    Bottom
};

class Text {
public:
    void Initialize(uint32_t fontHandle);
    void SetString(const std::u32string& text);
    void SetPosition(const Vector2& pos);
    void SetColor(const Vector4& color);
    void SetAlign(TextAlign align);
    void SetVerticalAlign(VerticalAlign align) { verticalAlign_ = align; }
    void SetBlendMode(const BlendMode& blendMode);
    void SetSize(const Vector2 size) { size_ = size; }
    void Draw();
    void Update(const bool isSound = false);
    void UpdateLayout(const bool isType = true);
    void StartTyping(float speed);
    const Vector2& GetPosition()const { return position_; }

private:
    uint32_t fontHandle_ = 0;
    std::u32string text_;
    Vector2 position_ = { 0.0f, 0.0f };
    Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

    Vector2 size_ = { 1280.0f,720.0f };
    std::vector<GlyphRun> glyphRuns_;
    std::vector<Font*> activeFonts_;
    VerticalAlign verticalAlign_ = VerticalAlign::Middle;
    TextAlign align_ = TextAlign::Center;
    BlendMode blendMode_ = kBlendModeAlpha;

    float typeTimer_ = 0.0f;         // 経過時間
    float typeSpeed_ = 0.05f;        // 1文字表示する間隔（秒）
    size_t visibleCharCount_ = 0;    // 現在表示する文字数
    bool isTyping_ = false;          // タイピング中かどうか
};
