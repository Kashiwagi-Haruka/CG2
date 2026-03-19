#include "TitleScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <imgui.h>
#include"Text/FreetypeManager/FreeTypeManager.h"
#include"Function.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

TitleScene::TitleScene() {

    BGMData_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/clock.mp3");
    Audio::GetInstance()->SetSoundVolume(&BGMData_, 1.0f);

    transition = std::make_unique<SceneTransition>();

    /// @brief 初期化
    FreeTypeManager::Initialize();
    Text::LoadSE();
    titleMenuUI_ = std::make_unique<TitleMenuUI>();
    firstStory_ = std::make_unique<FirstStory>();
}

void TitleScene::Finalize() {
    Audio::GetInstance()->SoundUnload(&BGMData_);
    Text::UnLoadSE();
}

void TitleScene::Initialize() {

    isBGMPlaying = false;
    isTransitionIn = true;
    isTransitionOut = false;
    transition->Initialize(false);
    titleMenuUI_->Initialize();
    firstStory_->Initialize();
}

void TitleScene::Update() {



    if (!isBGMPlaying) {
        Audio::GetInstance()->SoundPlayWave(BGMData_, true);
        isBGMPlaying = true;
    }

    titleMenuUI_->Update();

    if (titleMenuUI_->GetIsStart()) {
        firstStory_->Update();
    }

    if (firstStory_->GetIsEnd()) {
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


    Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(true);
    Object3dCommon::GetInstance()->SetVignetteStrength(true);

    titleMenuUI_->Draw();
    firstStory_->Draw();   
    FreeTypeManager::ResetFontUsage();
    SpriteCommon::GetInstance()->DrawCommon();

    //if (isTransitionIn || isTransitionOut) {
    //    transition->Draw();
    //}


}