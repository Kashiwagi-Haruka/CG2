#define NOMINMAX
#include "TutorialScene.h"
#include "CameraController/CameraController.h"
#include "Function.h"
#include "GameBase.h"
#include "Object/Background/SkyDome.h"
#include "Object/Character/Model/CharacterModel.h"
#include "Object/ExpCube/ExpCubeManager.h"
#include "Object/MapchipField.h"
#include "Object/Player/Player.h"
#include "Object3d/Object3dCommon.h"
#include "Pause/Pause.h"
#include "SceneManager.h"
#include "Sprite.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include "TutorialUI/TutorialUI.h"
#include <algorithm>
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
	tutorialUI_ = std::make_unique<TutorialUI>();
	expCubeManager_ = std::make_unique<ExpCubeManager>();

	cameraController_->Initialize();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraController_->GetCamera());

	skyDome_->Initialize(cameraController_->GetCamera());
	player_->Initialize(cameraController_->GetCamera());
	field_->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field_->Initialize(cameraController_->GetCamera());
	expCubeManager_->Initialize(cameraController_->GetCamera());
	pause_->Initialize();
	tutorialUI_->Initialize();

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
	stepTimer_ = 0.0f;
	expCubeCollectedCount_ = 0;
	expCubesSpawned_ = false;

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
		expCubeManager_->Update(cameraController_->GetCamera(), player_->GetMovementLimitCenter(), player_->GetMovementLimitRadius());
		if (expCubeManager_ && player_->GetIsAlive()) {
			for (auto& cube : expCubeManager_->GetCubes()) {
				if (!cube || cube->IsCollected()) {
					continue;
				}
				const Vector3 cubePos = cube->GetPosition();
				Vector3 toCube = player_->GetPosition() - cubePos;
				toCube.y = 0.0f;
				const Vector3 playerScale = player_->GetScale();
				const Vector3 cubeScale = cube->GetScale();
				const float playerRadius = std::max(playerScale.x, playerScale.z);
				const float cubeRadius = std::max(cubeScale.x, cubeScale.z);
				const float pickupRadius = playerRadius + cubeRadius;
				if (LengthSquared(toCube) <= pickupRadius * pickupRadius) {
					cube->Collect();
					player_->EXPMath();
				}
			}
		}
		player_->SetCamera(cameraController_->GetCamera());
		skyDome_->SetCamera(cameraController_->GetCamera());
		field_->SetCamera(cameraController_->GetCamera());

		if (!isTutorialComplete_) {
			bool progressed = false;
			switch (currentStepIndex_) {
			case 0:
				stepTimer_ += deltaTime;
				progressed = stepTimer_ >= kAutoAdvanceDuration;
				break;
			case 1:
				stepTimer_ += deltaTime;
				progressed = stepTimer_ >= kAutoAdvanceDuration;
				break;
			case 2:
				stepTimer_ = 0.0f;
				progressed = GameBase::GetInstance()->PushKey(DIK_W) || GameBase::GetInstance()->PushKey(DIK_A) || GameBase::GetInstance()->PushKey(DIK_S) || GameBase::GetInstance()->PushKey(DIK_D) ||
				             GameBase::GetInstance()->GetJoyStickLXY().x != 0.0f || GameBase::GetInstance()->GetJoyStickLXY().y != 0.0f;
				break;
			case 3:
				stepTimer_ = 0.0f;
				progressed = GameBase::GetInstance()->GetMouseMove().x != 0.0f || GameBase::GetInstance()->GetMouseMove().y != 0.0f || GameBase::GetInstance()->GetJoyStickRXY().x != 0.0f ||
				             GameBase::GetInstance()->GetJoyStickRXY().y != 0.0f;
				break;
			case 4:
				stepTimer_ = 0.0f;
				progressed = GameBase::GetInstance()->TriggerMouseButton(Input::MouseButton::kLeft) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonB);
				break;
			case 5:
				stepTimer_ = 0.0f;
				if (!expCubesSpawned_) {
					expCubeManager_->SpawnDrops(player_->GetPosition(), kExpCubeTargetCount);
					expCubesSpawned_ = true;
				}
				expCubeCollectedCount_ = 0;
				for (const auto& cube : expCubeManager_->GetCubes()) {
					if (cube && cube->IsCollected()) {
						expCubeCollectedCount_++;
					}
				}
				progressed = expCubeCollectedCount_ >= kExpCubeTargetCount;
				break;
			case 6:
				stepTimer_ = 0.0f;
				progressed = GameBase::GetInstance()->TriggerKey(DIK_E) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonY);
				break;
			case 7:
				stepTimer_ += deltaTime;
				progressed = stepTimer_ >= kEndAutoAdvanceDuration;
				break;
			default:
				stepTimer_ = 0.0f;
				break;
			}

			if (progressed) {
				stepCompleted_[currentStepIndex_] = true;
				currentStepIndex_++;
				stepTimer_ = 0.0f;
				if (currentStepIndex_ >= kStepCount) {
					isTutorialComplete_ = true;
				}
			}
		} else {
			SceneManager::GetInstance()->ChangeScene("Game");
			return;
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

	const int completedSteps = isTutorialComplete_ ? kStepCount : currentStepIndex_;
	const float tutorialProgress = static_cast<float>(completedSteps) / static_cast<float>(kStepCount);
	const float skipProgress = (skipHoldTimer_ >= kSkipHoldDuration) ? 1.0f : (skipHoldTimer_ / kSkipHoldDuration);
	if (tutorialUI_) {
		const int stepSpriteIndex = isTutorialComplete_ ? (kStepCount - 1) : currentStepIndex_;
		tutorialUI_->Update(tutorialProgress, skipProgress, stepSpriteIndex, skipHoldTimer_ > 0.0f);
	}

	if (isPaused_) {
		return;
	}

#ifdef USE_IMGUI
	ImGui::Begin("Tutorial");
	ImGui::Text("体験型チュートリアル");
	ImGui::Separator();
	const char* stepTitles[kStepCount] = {"開始", "準備", "移動", "視点移動", "攻撃", "ExpCube", "スキル", "おわり"};
	const char* stepHints[kStepCount] = {"チュートリアル開始", "準備ができたら進みます", "WASD で移動してみよう", "マウスで視点を動かしてみよう",
	                                     "左クリックで攻撃",   "ExpCube を4個集めよう",  "E でスキル攻撃",        "まもなくゲーム開始"};
	for (int i = 0; i < kStepCount; ++i) {
		ImVec4 color = stepCompleted_[i] ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		if (i == currentStepIndex_) {
			color = ImVec4(1.0f, 0.9f, 0.3f, 1.0f);
		}
		ImGui::TextColored(color, "%s: %s", stepTitles[i], stepHints[i]);
	}
	ImGui::Separator();
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
	if (expCubeManager_) {
		expCubeManager_->Draw();
	}

	SpriteCommon::GetInstance()->DrawCommon();
	if (controlSprite_) {
		controlSprite_->Draw();
	}
	if (tutorialUI_) {
		tutorialUI_->Draw();
	}
	pause_->Draw();
}

void TutorialScene::Finalize() {}