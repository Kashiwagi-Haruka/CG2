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
#include"GameObject/YoshidaMath/Easing.h"
#include"GameBase.h"
void TitleScene::CameraUpdate()
{
   const float deltaTime =  GameBase::GetInstance()->GetDeltaTime();
   cameraMoveTimer_ += deltaTime;
   if (cameraMoveTimer_ >= 1.0f) {
       cameraMoveTimer_ = 0.0f;
       cameraRandomOffset_.x = random_->Get();
       cameraRandomOffset_.y = random_->Get();
   }

    cameraTransform_.translate = YoshidaMath::Easing::Lerp(cameraTransform_.translate,cameraDefaultPos_+cameraRandomOffset_,0.01f);

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
}

void TitleScene::TransitionStart()
{
	if (!isTransitionOut) {
		transition->Initialize(true);
		isTransitionOut = true;
	}
}

TitleScene::TitleScene() {

	BGMData_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/SE/clock.mp3");
	Audio::GetInstance()->SetSoundVolume(&BGMData_, 1.0f);
	// カメラのインスタンス化
	camera_ = std::make_unique<Camera>();
	cameraDefaultPos_ = {0.0f, 0.0f, -0.3f};

	transition = std::make_unique<SceneTransition>();
	// ランダム
	random_ = std::make_unique<RandomClass>();

	titleMenuUI_ = std::make_unique<TitleMenuUI>();
	firstStory_ = std::make_unique<FirstStory>();
	gameContinued_ = std::make_unique<GameContinued>();
	// ゲームオブジェクト
	timeCard_ = std::make_unique<TimeCard>();
	timeCardRack_ = std::make_unique<TimeCardRack>();
	wall_ = std::make_unique<Wall>();
	identityMat_ = Function::MakeIdentity4x4();
}

void TitleScene::Finalize() { Audio::GetInstance()->SoundUnload(&BGMData_); }

void TitleScene::Initialize() {

	Audio::GetInstance()->SoundPlayWave(BGMData_, true);

	isTransitionIn = true;
	isTransitionOut = false;
	transition->Initialize(false);

	// ランダム
	random_->SetMinMax(-0.01f, 0.01f);

	titleMenuUI_->Initialize();
	firstStory_->Initialize();
	gameContinued_->Initialize();
	isGameContinuedOpen_ = false;

	// カメラ
	cameraTransform_ = {
	    .scale = {1.0f, 1.0f, 1.0f},
          .rotate = {0.0f, 0.0f, 0.0f},
          .translate = cameraDefaultPos_
    };
	// カメラ
	camera_->SetTransform(cameraTransform_);
	cameraRandomOffset_ = {0.0f};
	cameraMoveTimer_ = 0.0f;

	// ライト
	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 1.0f;
	// ゲームオブジェクト
	timeCard_->Initialize();
	timeCardRack_->Initialize();
	wall_->Initialize();
	wall_->SetParentMatrix(&identityMat_);
	wall_->SetST({14.0f, 4.0f, 1.0f}, {0.0f, 0.0f, 0.75f});
	timeCard_->SetCamera(camera_.get());
	timeCardRack_->SetCamera(camera_.get());
	wall_->SetCamera(camera_.get());
}

void TitleScene::Update() {

	if (!isGameContinuedOpen_) {
		titleMenuUI_->Update();

		if (titleMenuUI_->ConsumeContinueTriggered()) {
			isGameContinuedOpen_ = true;
		}
	} else {
		gameContinued_->Update();
	}

	if (titleMenuUI_->GetIsStart()) {

		firstStory_->Update();

		if (firstStory_->GetIsEnd()) {
			TransitionStart();
		}
	} else if (gameContinued_->GetIsSelected()) {
		TransitionStart();
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

	CameraUpdate();

	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);

	timeCard_->Update();
	timeCardRack_->Update();
	wall_->Update();
}
void TitleScene::Draw() {

	Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(true);
	Object3dCommon::GetInstance()->SetVignetteStrength(true);

	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	Object3dCommon::GetInstance()->DrawCommon();

	timeCard_->Draw();
	timeCardRack_->Draw();
	wall_->Draw();

	SpriteCommon::GetInstance()->DrawCommonFont();
	if (!isGameContinuedOpen_) {
		titleMenuUI_->Draw();
	} else {
		gameContinued_->Draw();
	}
	firstStory_->Draw();
	FreeTypeManager::ResetFontUsage();
	SpriteCommon::GetInstance()->DrawCommon();

	if (isTransitionIn || isTransitionOut) {
		transition->Draw();
	}
}