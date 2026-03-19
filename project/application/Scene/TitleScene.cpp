#include "TitleScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <imgui.h>
#include"FreetypeManager/FreeTypeManager.h"
#include"Function.h"


TitleScene::TitleScene() {

    BGMData_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/clock.mp3");
    Audio::GetInstance()->SetSoundVolume(&BGMData_, 1.0f);

    transition = std::make_unique<SceneTransition>();
    titleMenuUI_ = std::make_unique<TitleMenuUI>();
}

void TitleScene::Finalize() {
    Audio::GetInstance()->SoundUnload(&BGMData_);

}


void TitleScene::Initialize() {

    isBGMPlaying = false;
    isTransitionIn = true;
    isTransitionOut = false;
    transition->Initialize(false);
    titleMenuUI_->Initialize();
}

void TitleScene::Update() {
    if (!isBGMPlaying) {
        Audio::GetInstance()->SoundPlayWave(BGMData_, true);
        isBGMPlaying = true;
    }


    titleMenuUI_->Update();

    if (titleMenuUI_->GetIsStart()) {
        if (!isTransitionOut) {
            transition->Initialize(true);
            isTransitionOut = true;
        }
    }

    if (isTransitionIn || isTransitionOut) {
        transition->Update();
        if (transition->IsEnd() && isTransitionIn) {
            isTransitionIn = false;
        }
        if (transition->IsEnd() && isTransitionOut) {
            SceneManager::GetInstance()->ChangeScene("ShadowGame");
        }
    }
}
void TitleScene::Draw() {

    titleMenuUI_->Draw();

    SpriteCommon::GetInstance()->DrawCommon();

    if (isTransitionIn || isTransitionOut) {
        transition->Draw();
    }


}