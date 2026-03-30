#pragma once
#include<memory>
#include"Sprite.h"

class LightIcon
{
public:
    LightIcon();
    void Initialize();
    void Update();
    void Draw();
private:
    std::unique_ptr<Sprite> sprite_ = nullptr;
    uint32_t handle_ = 0;
};

