#pragma once
#include <vector>
#include<cstdint>
#include <string>

class GentlemanTalk
{
    static uint32_t talkNum_;
    static bool isSendMessage_;
    static bool isDraw_;
    static std::vector<std::u32string>strings_;
public:
    GentlemanTalk();
    ~GentlemanTalk();
    void SetStrings(const std::string& stageName);
    void Initialize();
    void Update();
    void GentlemanSendMessage(const bool random = false);
    static bool GetIsSendMessage() { return isSendMessage_; }
    static bool GetIsDraw() { return isDraw_; }
    void SetIsDraw(bool isDraw) { isDraw_  = isDraw; }
    static std::u32string& GetString() {

        if (talkNum_ < strings_.size()) {
            return strings_.at(talkNum_);
        }

        static std::u32string string = U"今日の天気は晴天です。";
        return string;
    }
};

