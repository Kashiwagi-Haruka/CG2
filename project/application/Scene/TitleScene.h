#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <memory>
#include"Text/TItleMenu/TitleMenuUI.h"
#include"Text/FirstStory/FirstStory.h"
#include"GameObject/TimeCard/TimeCard.h"
#include"GameObject/TimeCard/TimeCardRack.h"
#include "Light/DirectionalLight.h" 
#include"Camera.h"
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

    std::unique_ptr<TimeCard> timeCard_ = nullptr;
    std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;
    std::unique_ptr<Camera> camera_ = nullptr;
    Transform cameraTransform_;
    //DirectionalLight
    DirectionalLight directionalLight_{};
public:
    TitleScene();
    ~TitleScene() override = default;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
};
