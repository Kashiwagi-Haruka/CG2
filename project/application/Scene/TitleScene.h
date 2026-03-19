#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <memory>
#include"Text/TItleMenu/TitleMenuUI.h"
#include"Text/FirstStory/FirstStory.h"

class GameBase;

class TitleScene : public BaseScene {
private:
    SoundData BGMData_;
    bool isBGMPlaying;
    bool isTransitionIn = false;
    bool isTransitionOut = false;
    std::unique_ptr<SceneTransition> transition = nullptr;
    std::unique_ptr<TitleMenuUI> titleMenuUI_ = nullptr;
    std::unique_ptr<FirstStory> firstStory_ = nullptr;
public:
    TitleScene();
    ~TitleScene() override = default;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
};
