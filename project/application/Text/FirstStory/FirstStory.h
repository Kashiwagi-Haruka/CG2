#pragma once
#include"Text/Text.h"
#include<array>
class FirstStory
{
public:
    FirstStory();
    void Initialize();
    void Update();
    void Draw();
    bool GetIsEnd() { return  isEnd_; };
private:
    bool isEnd_ = true;
    uint32_t shotCount_ = 0;
   uint32_t fontHandle_ = 0;
    Text text_;
    std::u32string strings_;
};

