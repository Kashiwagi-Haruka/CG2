#include"Text/Text.h"
#include<array>
#include <memory>
#include"GameObject/YoshidaMath/RandomClass.h"
#include "Audio.h"

class ChairMenu
{
public:

    enum MENU {
        GRAB_TEXT,
        STAND_TEXT,
        CLOSE_TEXT,
        MAX_TEXT,
    };
public:

    //テキストの設定
    ChairMenu();
    ~ChairMenu();
    void Initialize();
    void Update();
    void Draw();
    static void SetIsShowMenu(const bool isShowMenu) { isShowMenu_ = isShowMenu; }
    static bool GetIsShowMenu() { return isShowMenu_; }
    static uint32_t GetSelectButtonNum() { return selectButtonNum_; }
    static void SetIsSelectButton(const bool isSelectButton) { isSelectButton_ = isSelectButton; }
    static bool GetIsSelectButton() { return isSelectButton_; }
private:
#pragma region //Text
    uint32_t menuFontHandle_;

    Text triangleText_;
    Text pressEText_;
    std::array<Text, MAX_TEXT> menuText_;
    float fontTheta_ = 0.0f;
    std::array< std::u32string, 2> grabText_;

#pragma endregion
    static bool isShowMenu_;
    static uint32_t selectButtonNum_;
    static bool isSelectButton_;
};

