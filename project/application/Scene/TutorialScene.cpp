#include "TutorialScene.h"
#include "CameraController/CameraController.h"
#include "GameBase.h"
#include "Object/Background/SkyDome.h"
#include "Object/Character/Model/CharacterModel.h"
#include "Object/MapchipField.h"
#include "Object/Player/Player.h"
#include "Object3d/Object3dCommon.h"
#include "Pause/Pause.h"
#include "SceneManager.h"
#include "Sprite.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

TutorialScene::TutorialScene() {}

void TutorialScene::Initialize() {
	CharacterModel characterModel;
	characterModel.LoadModel();
	cameraController_ = std::make_unique<CameraController>();
	skyDome_ = std::make_unique<SkyDome>();
	player_ = std::make_unique<Player>();
	field_ = std::make_unique<MapchipField>();
	pause_ = std::make_unique<Pause>();

	cameraController_->Initialize();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraController_->GetCamera());

	skyDome_->Initialize(cameraController_->GetCamera());
	player_->Initialize(cameraController_->GetCamera());
	field_->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field_->Initialize(cameraController_->GetCamera());
	pause_->Initialize();

	controlSpriteHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/option.png");
	controlSprite_ = std::make_unique<Sprite>();
	controlSprite_->Initialize(controlSpriteHandle_);
	controlSprite_->SetScale({700, 100});
	controlSprite_->SetPosition({10, 600});
	controlSprite_->Update();

	stepCompleted_.fill(false);
	currentStepIndex_ = 0;
	isTutorialComplete_ = false;
	isPaused_ = false;
	wasSkipKeyHeld_ = false;
	skipHoldTimer_ = 0.0f;

	GameBase::GetInstance()->SetIsCursorStablity(true);
	GameBase::GetInstance()->SetIsCursorVisible(false);
}

void TutorialScene::Update() {
	const float deltaTime = 1.0f / 60.0f;
	bool skipKeyHeld = GameBase::GetInstance()->PushKey(DIK_P);
	if (GameBase::GetInstance()->TriggerKey(DIK_ESCAPE) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonStart)) {
		isPaused_ = !isPaused_;
	}
	if (skipKeyHeld) {
		skipHoldTimer_ += deltaTime;
		if (skipHoldTimer_ >= kSkipHoldDuration) {
			SceneManager::GetInstance()->ChangeScene("Game");
			return;
		}
	} else {
		if (wasSkipKeyHeld_ && skipHoldTimer_ < kSkipHoldDuration) {
			isPaused_ = !isPaused_;
		}
		skipHoldTimer_ = 0.0f;
	}
	wasSkipKeyHeld_ = skipKeyHeld;

	if (!isPaused_) {
		cameraController_->SetPlayerPos(player_->GetPosition());
		cameraController_->Update();
		skyDome_->Update();
		field_->Update();
		player_->Update();
		player_->SetCamera(cameraController_->GetCamera());
		skyDome_->SetCamera(cameraController_->GetCamera());
		field_->SetCamera(cameraController_->GetCamera());

		if (!isTutorialComplete_) {
			bool progressed = false;
			switch (currentStepIndex_) {
			case 0:
				progressed = GameBase::GetInstance()->PushKey(DIK_W) || GameBase::GetInstance()->PushKey(DIK_A) || GameBase::GetInstance()->PushKey(DIK_S) || GameBase::GetInstance()->PushKey(DIK_D);
				break;
			case 1:
				progressed = GameBase::GetInstance()->PushKey(DIK_LSHIFT) || GameBase::GetInstance()->PushKey(DIK_RSHIFT) || GameBase::GetInstance()->PushButton(Input::PadButton::kButtonLeftShoulder);
				break;
			case 2:
				progressed = GameBase::GetInstance()->TriggerKey(DIK_SPACE) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonA);
				break;
			case 3:
				progressed = GameBase::GetInstance()->TriggerMouseButton(Input::MouseButton::kLeft) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonB);
				break;
			case 4:
				progressed = GameBase::GetInstance()->TriggerKey(DIK_E) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonY);
				break;
			case 5:
				progressed = GameBase::GetInstance()->TriggerKey(DIK_Q) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonX);
				break;
			default:
				break;
			}

			if (progressed) {
				stepCompleted_[currentStepIndex_] = true;
				currentStepIndex_++;
				if (currentStepIndex_ >= kStepCount) {
					isTutorialComplete_ = true;
				}
			}
		} else {
			if (GameBase::GetInstance()->TriggerKey(DIK_SPACE) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonA)) {
				SceneManager::GetInstance()->ChangeScene("Game");
			}
		}
	}
	pause_->Update(isPaused_);
	Pause::Action pauseAction = pause_->ConsumeAction();
	if (pauseAction == Pause::Action::kResume) {
		isPaused_ = false;
	} else if (pauseAction == Pause::Action::kTitle) {
		SceneManager::GetInstance()->ChangeScene("Title");
		return;
	}

	if (isPaused_) {
		return;
	}

#ifdef USE_IMGUI
	ImGui::Begin("Tutorial");
	ImGui::Text("体験型チュートリアル");
	ImGui::Separator();
	const char* stepTitles[kStepCount] = {"移動", "ダッシュ", "ジャンプ", "通常攻撃", "スキル", "必殺技"};
	const char* stepHints[kStepCount] = {"WASD で移動してみよう", "SHIFT を押してダッシュ", "SPACE でジャンプ", "左クリックで攻撃", "E でスキル攻撃", "Q で必殺技"};
	for (int i = 0; i < kStepCount; ++i) {
		ImVec4 color = stepCompleted_[i] ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		if (i == currentStepIndex_) {
			color = ImVec4(1.0f, 0.9f, 0.3f, 1.0f);
		}
		ImGui::TextColored(color, "%s: %s", stepTitles[i], stepHints[i]);
	}
	ImGui::Separator();
	if (isTutorialComplete_) {
		ImGui::Text("SPACE でゲーム開始");
	}
	ImGui::Text("P 長押しでチュートリアルスキップ");
	ImGui::Text("P 押しでポーズ/再開");
	if (isPaused_) {
		ImGui::Text("PAUSE");
	}
	ImGui::End();
#endif // USE_IMGUI
}

void TutorialScene::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	skyDome_->Draw();
	field_->Draw();
	player_->Draw();

	SpriteCommon::GetInstance()->DrawCommon();
	if (controlSprite_) {
		controlSprite_->Draw();
	}
	pause_->Draw();
}

void TutorialScene::Finalize() {}