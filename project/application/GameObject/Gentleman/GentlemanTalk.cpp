#include "GentlemanTalk.h"
#include "Engine/Loadfile/TXT/TxtManager.h"
#include"GameObject/KeyBindConfig.h"

size_t GentlemanTalk::talkNum_ = 0;
bool GentlemanTalk::isSendMessage_ = false;
bool GentlemanTalk:: isDraw_ = false;
bool GentlemanTalk::isTalkEnd_ = true;

std::vector<std::u32string>GentlemanTalk::strings_;

void GentlemanTalk::SetStrings(const std::string& stageName) {

    Initialize();

    std::u32string string;

    std::string filePath = "Resources/TXT/gentlemanTalk" + stageName + ".txt";
    try {
        string = TxtManager::GetInstance()->LoadTxtAsU32String(filePath.c_str());
    } catch (...) {
        string = U"gentlemanTalk.txt の読み込みに失敗しました。";
    }

    // 検索の開始位置と、見つかったセミコロンの位置を管理する変数
    std::u32string::size_type start = 0;
    std::u32string::size_type end = string.find(U';');

    while (end != std::u32string::npos) {
        // セミコロンの直前までの文字列を切り出して追加
        strings_.push_back(string.substr(start, end - start));

        // 次の検索開始位置を、セミコロンの次の文字に進める
        start = end + 2;

        // 次のセミコロンを検索
        end = string.find(U';', start);
    }

};

GentlemanTalk::GentlemanTalk()
{
    //最初はコレ
    SetStrings("");
}

GentlemanTalk::~GentlemanTalk()
{
    strings_.clear();
}

void GentlemanTalk::Initialize()
{
    strings_.clear();
    talkNum_ = strings_.size() - 1;
    isSendMessage_ = false;
    isTalkEnd_ = true;
}

void GentlemanTalk::Update()
{
    isSendMessage_ = false;
}

void GentlemanTalk::GentlemanSendMessage(const bool random)
{
    if (PlayerCommand::GetInstance()->InteractTrigger()) {
        isDraw_ = true;
        isSendMessage_ = true;
        isTalkEnd_ = false;
        if (random) {
            talkNum_ = rand() % strings_.size();
        } else {

            if (talkNum_ < strings_.size() - 1) {
                talkNum_++;
            } else {
                talkNum_ = 0;
            }
        }
    }
}
