#pragma once

#include"Audio.h"
#include<array>
class EdamameTrivia
{
private:
    SoundData triviaVoice_;
    std::array<std::string, 8> strings_;
    size_t triviaNum_ = 0;
public:
    EdamameTrivia();
    ~EdamameTrivia();
    void Initialize();
    void Update();
    void Draw();
    void SetVol(float vol);
};

