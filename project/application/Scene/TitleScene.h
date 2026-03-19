#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <memory>
#include"FreetypeManager/Text.h"
#include<array>
class GameBase;

class TitleScene : public BaseScene {


    SoundData BGMData_;
    SoundData SEData_;
    bool isBGMPlaying;
    bool isTransitionIn = false;
    bool isTransitionOut = false;
    std::unique_ptr<SceneTransition> transition = nullptr;
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

#pragma endregion

    bool isShowMenu_ = false;
    bool isSelectButton_ = false;
    uint32_t selectButtonNum_ = 0;
public:
    TitleScene();
    ~TitleScene() override = default;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
private:
    //テキストの設定
    void FirstSettingText();
};
