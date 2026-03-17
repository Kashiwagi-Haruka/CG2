#pragma once
#include"Text.h"

class TextUIManager
{
public:
    TextUIManager();
    ~TextUIManager();
    void Initialize();
    void Update();
    void Draw();
private:
    uint32_t fontHandle_;
    Text text_;
    const float showTime_ = 1.0f;
    float showTimer_ = showTime_;
    bool isDraw_ = false;
};

