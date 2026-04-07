#pragma once
#include"Text/Text.h"
#include<array>
#include <memory>
#include"GameObject/YoshidaMath/RandomClass.h"
#include "Audio.h"

class GentlemanMenu
{
public:
    enum MenuType { TALK, SAVE,CANCEL, MAX_TEXT};
public:

    //テキストの設定
    GentlemanMenu();
    ~GentlemanMenu();
    void Initialize();
    void Update();
    void Draw();
    static void SetIsShowMenu(const bool isShowMenu) { isShowMenu_ = isShowMenu; }
    static bool GetIsShowMenu() { return isShowMenu_; }
    static uint32_t GetSelectButtonNum() { return selectButtonNum_; }
private:
#pragma region //Text
    uint32_t menuFontHandle_;
    Text triangleText_;
    Text pressEText_;
    std::array<Text, MAX_TEXT> menuText_;
    float fontTheta_ = 0.0f;
#pragma endregion
    static bool isShowMenu_;
    static uint32_t selectButtonNum_;
    bool isShowStart_ = false;
};

