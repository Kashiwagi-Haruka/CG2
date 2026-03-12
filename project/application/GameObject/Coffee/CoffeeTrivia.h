#pragma once

#include"Audio.h"
#include<array>
class CoffeeTrivia
{
private:
    SoundData triviaVoice_;
    std::array<std::string, 6> strings_;
    uint32_t triviaNum_ = 0;
public:
    CoffeeTrivia();
    ~CoffeeTrivia();
    void Initialize();
    void Update();
    void Draw();
    void SetVol(float vol);
};