#pragma once
#include"Text/Text.h"
#include"Text/ChairMenu/ChairMenu.h"
#include<memory>

class TextUIManager
{
public:
    TextUIManager();
    ~TextUIManager();
    void Initialize();
    void Update();
    void Draw();
private:
    std::unique_ptr<ChairMenu>chairMenu_ = nullptr;
    uint32_t fontHandle_;
    Text text_;
    Text edamameTrivia_;
    const float showTime_ = 1.0f;
    float showTimer_ = showTime_;
    bool isDraw_ = false;

};

