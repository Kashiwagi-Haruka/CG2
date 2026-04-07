#pragma once
#include "Sprite.h"
#include "Vector2.h"
#include <memory>
#include<array>
class RaySprite
{
public:
    enum TextureUI {
        PORTAL,
        HAND,
        GRAB,
        TALK,
        MAX
    };
    RaySprite();
    void Initialize();
    void Update();
    void Draw();
    void SetTexture(const TextureUI num);
private:
    bool IsRayHit();
private:
    std::unique_ptr<Sprite> sprite_ = nullptr;
    std::array<uint32_t, MAX>handle_;

};

