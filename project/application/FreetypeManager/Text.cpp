#define NOMINMAX
#include "Text.h"
#include"imgui.h"
#include"WinApp.h"
#include"SpriteCommon.h"
#include"DirectXCommon.h"

void Text::Initialize(uint32_t fontHandle) {
    fontHandle_ = fontHandle;
    size_ = { static_cast<float>(WinApp::kClientWidth),static_cast<float>(WinApp::kClientHeight) };
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

    std::u32string visibleText = text_.substr(0, visibleCharCount_);

    // まず仮レイアウト（位置は0始まり）を取得
    //glyphRuns_ = FreeTypeManager::LayoutString(fontHandle_, text_, { 0.0f, 0.0f }, size_.x);
    glyphRuns_ = FreeTypeManager::LayoutString(fontHandle_, visibleText, { 0.0f, 0.0f }, size_.x);
    // 中央・右寄せ対応（行ごとにオフセット）
    std::unordered_map<float, float> lineWidths;
    for (const auto& run : glyphRuns_) {
        auto& size = FreeTypeManager::GetGlyphTextures({ fontHandle_, run.glyphIndex }).glyphSize;
        lineWidths[run.position.y] += size.y;
    }

    for (auto& run : glyphRuns_) {
        float offsetX = 0.0f;
        float lineWidth = lineWidths[run.position.y];
        switch (align_) {
        case TextAlign::Center:
            offsetX = -lineWidth * 0.5f;
            break;
        case TextAlign::Right:
            offsetX = -lineWidth;
            break;
        default:
            break;
        }
        run.position.x += position_.x + offsetX;
        run.position.y += position_.y;
    }
}

void Text::StartTyping(float speed)
{
    typeSpeed_ = speed;
    typeTimer_ = 0.0f;
    visibleCharCount_ = 0;
    isTyping_ = true;
    UpdateLayout(); // レイアウト更新
}

void Text::Draw() {

    activeFonts_.clear();

    for (const auto& run : glyphRuns_) {
        GlyphKey key{ fontHandle_, run.glyphIndex };
        auto* font = FreeTypeManager::GetOrCreateFont(key);
        auto& texData = FreeTypeManager::GetGlyphTextures(key);
        // ベースラインに合わせてY位置を調整！
        float y = run.position.y - (texData.glyphSize.y + texData.bearingY) / 2.0f;
        float x = run.position.x;
        font->SetPosition({ x,y });
        font->SetColor(color_);
        font->Update();
        font->Draw();
        activeFonts_.push_back(font);
    }

}

void Text::Update()
{
    if (isTyping_) {
        const float deltaTime = SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        typeTimer_ += deltaTime;
        while (typeTimer_ >= typeSpeed_) {
            typeTimer_ -= typeSpeed_;
            if (visibleCharCount_ < text_.size()) {
                visibleCharCount_++;
                UpdateLayout(); // レイアウト更新
            } else {
                isTyping_ = false; // 全部表示し終わったら止める
                break;
            }
        }
    }
}
