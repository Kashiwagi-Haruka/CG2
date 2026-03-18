#pragma once

#include"Audio.h"
#include<array>
class EdamameTrivia
{
private:
    SoundData triviaVoice_;
    static std::array<std::u32string, 8> strings_;
    static size_t triviaNum_;
    static bool isSendStartTriviaMessage_;
public:
    EdamameTrivia();
    ~EdamameTrivia();
    void Initialize();
    void Update();
    void Draw();
    void SetVol(float vol);
    static bool GetIsSendStartTriviaMessage() { return  isSendStartTriviaMessage_; }
    static std::u32string& GetString() { return strings_[triviaNum_]; }
};

