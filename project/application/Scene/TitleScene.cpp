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
    //カメラのインスタンス化
    camera_ = std::make_unique<Camera>();

    transition = std::make_unique<SceneTransition>();

    /// @brief 初期化
    FreeTypeManager::Initialize();
    Text::LoadSE();
    titleMenuUI_ = std::make_unique<TitleMenuUI>();
    firstStory_ = std::make_unique<FirstStory>();

    timeCard_ = std::make_unique<TimeCard>();
    timeCardRack_ = std::make_unique<TimeCardRack>();

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

    cameraTransform_ = {
        .scale = {1.0f,1.0f,1.0f},
        .rotate = {0.0f,0.0f,0.0f},
        .translate = {0.0f,0.0f,-0.3f}
    };

    camera_->SetTransform(cameraTransform_);


    directionalLight_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLight_.direction = { 0.0f, -1.0f, 0.0f };
    directionalLight_.intensity = 1.0f;


    timeCard_->Initialize();
    timeCardRack_->Initialize();
    timeCard_->SetCamera(camera_.get());
    timeCardRack_->SetCamera(camera_.get());
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

    //カメラ
    camera_->SetTransform(cameraTransform_);
    camera_->Update();

#ifdef USE_IMGUI
    ImGui::Begin("Camera");
    ImGui::DragFloat3("scale", &cameraTransform_.scale.x);
    ImGui::DragFloat3("rotate", &cameraTransform_.rotate.x);
    ImGui::DragFloat3("translate", &cameraTransform_.translate.x);
    ImGui::End();
#endif

    Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);

    timeCard_->Update();
    timeCardRack_->Update();

}
void TitleScene::Draw() {

    Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(true);
    Object3dCommon::GetInstance()->SetVignetteStrength(true);

    Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
    Object3dCommon::GetInstance()->DrawCommon();
    
    timeCard_->Draw();
    timeCardRack_->Draw();

    SpriteCommon::GetInstance()->DrawCommonFont();
    titleMenuUI_->Draw();
    firstStory_->Draw();
    FreeTypeManager::ResetFontUsage();
    SpriteCommon::GetInstance()->DrawCommon();

    //if (isTransitionIn || isTransitionOut) {
    //    transition->Draw();
    //}


}