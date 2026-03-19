#pragma once

#include"Text/Text.h"
#include<array>
#include <memory>
#include"GameObject/YoshidaMath/RandomClass.h"
#include "Audio.h"

class TitleMenuUI
{
public:
    //テキストの設定
   TitleMenuUI();
   ~TitleMenuUI();
   void Initialize();
   void Update();
   void Draw();
   bool GetIsStart() { return isStart_; }
private:

    SoundData SEData_;

#pragma region //Text

    enum MENU {
        START_TEXT,
        CONTINUE_TEXT,
        OPTION_TEXT,
        MAX_TEXT,
    };

    uint32_t fontHandle_;
    uint32_t menuFontHandle_;

    Text titleText_;
    Text triangleText_;
    Text pressSpaceText_;
    std::array<Text, MAX_TEXT> menuText_;
    float fontTheta_ = 0.0f;

    std::unique_ptr<RandomClass> random_ = nullptr;
    Vector2 titleDefaultPos_ = { 0.0f };
#pragma endregion

    bool isShowMenu_ = false;
    bool isSelectButton_ = false;
    bool isStart_ = false;
    uint32_t selectButtonNum_ = 0;
};

