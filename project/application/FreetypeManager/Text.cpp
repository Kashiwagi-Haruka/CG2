#define NOMINMAX
#include "Text.h"
#include"imgui.h"

void Text::Initialize(uint32_t fontHandle) {
    fontHandle_ = fontHandle;
}

void Text::SetString(const std::u32string& text) {
    text_ = text;
}

void Text::SetPosition(const Vector2& pos) {
    position_ = pos;
}

void Text::SetColor(const Vector4& color) {
    color_ = color;
}

void Text::SetAlign(TextAlign align) {
    align_ = align;
}

void Text::SetBlendMode(const BlendMode& blendMode) {
    blendMode_ = blendMode;
}

void Text::UpdateLayout() {

    // まず仮レイアウト（位置は0始まり）を取得
    glyphRuns_ = FreeTypeManager::LayoutString(fontHandle_, text_, { 0.0f, 0.0f });

    // 中央・右寄せ対応
    float totalWidth = 0.0f;
    if (!glyphRuns_.empty()) {
        const auto& last = glyphRuns_.back();
        totalWidth = last.position.x - glyphRuns_.front().position.x;
    }

    float offsetX = 0.0f;
    switch (align_) {
    case TextAlign::Center:
        offsetX = -totalWidth * 0.5f;
        break;
    case TextAlign::Right:
        offsetX = -totalWidth;
        break;
    default:
        break;
    }


    // 最終的な位置を加算
    for (auto& run : glyphRuns_) {
        run.position.x += position_.x + offsetX;
        run.position.y += position_.y;
    }
}


void Text::Draw() {

    activeFonts_.clear();
 
    for (const auto& run : glyphRuns_) {
        GlyphKey key{ fontHandle_, run.glyphIndex };
        auto* font = FreeTypeManager::GetOrCreateFont(key);
        auto& texData = FreeTypeManager::GetGlyphTextures(key);
        // ベースラインに合わせてY位置を調整！
        float y = run.position.y - (texData.glyphSize.y + texData.bearingY) / 2.0f;
        font->SetPosition({ run.position.x,y });
        font->SetColor(color_);
        font->Update();
        font->Draw();
        activeFonts_.push_back(font);
    }

}