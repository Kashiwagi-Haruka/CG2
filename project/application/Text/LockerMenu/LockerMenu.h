#pragma once

#include"Text/Text.h"
#include<array>

class LockerMenu
{
public:

    //テキストの設定
    LockerMenu();
    ~LockerMenu();
    void Initialize();
    void Update();
    void Draw();
    static void SetIsShowMenu(const bool isShowMenu) { isShowMenu_ = isShowMenu; }
    static bool GetIsShowMenu() { return isShowMenu_; }
private:
#pragma region //Text
    uint32_t menuFontHandle_;
    Text triangleText_;
    Text exitText_;
    float fontTheta_ = 0.0f;
#pragma endregion
    static bool isShowMenu_;
    bool isShowStart_ = false;

};

