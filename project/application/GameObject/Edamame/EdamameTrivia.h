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
    static bool isDraw_;
public:
    EdamameTrivia();
    ~EdamameTrivia();
    void Initialize();
    void Update();
    void SetSound();
    void SetVol(float vol);
    static bool GetIsDraw() { return isDraw_; }
    static void SetIsDraw(const bool isDraw) { isDraw_ = isDraw; }
    static bool GetIsSendStartTriviaMessage() { return  isSendStartTriviaMessage_; }
    static std::u32string& GetString() { return strings_[triviaNum_]; }
};

