#include "GameScene.h"
#include "CameraController/CameraController.h"
#include "GameTimer/GameTimer.h"
#include "Model/ModelManager.h"
#include "Object/Background/SkyDome.h"
#include "Object/Enemy/EnemyManager.h"
#include "Object/ExpCube/ExpCubeManager.h"
#include "Object/Player/Player.h"
#include "Object3d/Object3dCommon.h"
#include "ParticleManager.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <numbers>

GameScene::GameScene() {
	characterModel.LoadModel();
	cameraController = std::make_unique<CameraController>();
	particles = std::make_unique<Particles>();
	skyDome = std::make_unique<SkyDome>();
	player = std::make_unique<Player>();
	enemyManager = std::make_unique<EnemyManager>();
	expCubeManager = std::make_unique<ExpCubeManager>();

	field = std::make_unique<MapchipField>();
	sceneTransition = std::make_unique<SceneTransition>();
	uimanager = std::make_unique<UIManager>();
	/*BG = std::make_unique<Background>();*/
	house = std::make_unique<House>();

	pause = std::make_unique<Pause>();
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Tailshaft.mp3");
	Audio::GetInstance()->SetSoundVolume(&BGMData, 0.3f);
	GameTimer::GetInstance()->Reset();
	GameBase::GetInstance()->SetIsCursorStablity(true);
	GameBase::GetInstance()->SetIsCursorVisible(false);
}

GameScene::~GameScene() {}

void GameScene::Finalize() {

	Audio::GetInstance()->SoundUnload(&BGMData);

	// level up icons
	for (int i = 0; i < 4; i++) {
		levelupIcons[i].reset();
	}
}

void GameScene::Initialize() {

	sceneEndClear = false;
	sceneEndOver = false;
	isBGMPlaying = false;
	cameraController->Initialize();

	Object3dCommon::GetInstance()->SetDefaultCamera(cameraController->GetCamera());

	skyDome->Initialize(cameraController->GetCamera());
	player->Initialize(cameraController->GetCamera());

	enemyManager->Initialize(cameraController->GetCamera());
	expCubeManager->Initialize(cameraController->GetCamera());
	field->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field->Initialize(cameraController->GetCamera());
	sceneTransition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	nextSceneName.clear();
	uimanager->SetPlayerHPMax(player->GetHPMax());
	uimanager->SetPlayerHP(player->GetHP());

	uimanager->Initialize();
	house->Initialize(cameraController->GetCamera());
	// ==================
	// ★ レベルアップ用スプライト画像読み込み
	// ==================

	int handle[4]{};
	handle[0] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_attack.png");
	handle[1] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_speed.png");
	handle[2] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_hp.png");
	handle[3] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_allow.png");

	for (int i = 0; i < 4; i++) {

		levelupIcons[i] = std::make_unique<Sprite>();
		levelupIcons[i]->Initialize(handle[i]);
		levelupIcons[i]->SetScale({256, 256});
	}

	uint32_t phaseHandles[5] = {
	    TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/phase1.png"), TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/phase2.png"),
	    TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/phase3.png"), TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/phase4.png"),
	    TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/phase5.png"),
	};

	for (int i = 0; i < 5; i++) {
		phaseSprites_[i] = std::make_unique<Sprite>();
		phaseSprites_[i]->Initialize(phaseHandles[i]);
		phaseSprites_[i]->SetScale(phaseSpriteSize_);
	}

	lastWave_ = 0;
	isPhaseSpriteActive_ = false;
	isPhaseSpritePaused_ = false;

	activePointLightCount_ = 3;
	pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[0].position = {-75.0f, 5.0f, -75.0f};
	pointLights_[0].intensity = 1.0f;
	pointLights_[0].radius = 10.0f;
	pointLights_[0].decay = 0.7f;
	pointLights_[1].color = {1.0f, 0.9f, 0.9f, 1.0f};
	pointLights_[1].position = {75.0f, 5.0f, 75.0f};
	pointLights_[1].intensity = 0.0f;
	pointLights_[1].radius = 10.0f;
	pointLights_[1].decay = 0.7f;
	pointLights_[2].color = {0.4f, 0.4f, 1.0f, 1.0f};
	pointLights_[2].position = {-75.0f, 5.0f, 75.0f};
	pointLights_[2].intensity = 1.0f;
	pointLights_[2].radius = 5.0f;
	pointLights_[2].decay = 0.7f;

	directionalLight_.color = {0.3725f, 0.2667f, 0.7882f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.5f};
	directionalLight_.intensity = 1.0f;

	activeSpotLightCount_ = 1;
	spotLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[0].position = {-50.0f, 5.0f, -50.0f};
	spotLights_[0].direction = {0.0f, 1.0f, 0.0f};
	spotLights_[0].intensity = 0.0f;
	spotLights_[0].distance = 7.0f;
	spotLights_[0].decay = 2.0f;
	spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
	pause->Initialize();
}

void GameScene::DebugImGui() {

#ifdef USE_IMGUI
	if (ImGui::Begin("SampleLight")) {
		if (ImGui::TreeNode("DirectionalLight")) {
			ImGui::ColorEdit4("LightColor", &directionalLight_.color.x);
			ImGui::DragFloat3("LightDirection", &directionalLight_.direction.x, 0.1f, -1.0f, 1.0f);
			ImGui::DragFloat("LightIntensity", &directionalLight_.intensity, 0.1f, 0.0f, 10.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PointLight")) {

			for (uint32_t index = 0; index < activePointLightCount_; ++index) {
				ImGui::PushID(static_cast<int>(index));
				if (ImGui::TreeNode("PointLight")) {
					ImGui::ColorEdit4("PointLightColor", &pointLights_[index].color.x);
					ImGui::DragFloat("PointLightIntensity", &pointLights_[index].intensity, 0.1f);
					ImGui::DragFloat3("PointLightPosition", &pointLights_[index].position.x, 0.1f);
					ImGui::DragFloat("PointLightRadius", &pointLights_[index].radius, 0.1f);
					ImGui::DragFloat("PointLightDecay", &pointLights_[index].decay, 0.1f);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SpotLight")) {
			ImGui::ColorEdit4("SpotLightColor", &spotLights_[0].color.x);
			ImGui::DragFloat("SpotLightIntensity", &spotLights_[0].intensity, 0.1f);
			ImGui::DragFloat3("SpotLightPosition", &spotLights_[0].position.x, 0.1f);
			ImGui::DragFloat3("SpotLightDirection", &spotLights_[0].direction.x, 0.1f);
			ImGui::DragFloat("SpotLightDistance", &spotLights_[0].distance, 0.1f);
			ImGui::DragFloat("SpotLightDecay", &spotLights_[0].decay, 0.1f);
			ImGui::DragFloat("SpotLightCosAngle", &spotLights_[0].cosAngle, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("SpotLightCosFalloffStart", &spotLights_[0].cosFalloffStart, 0.1f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
	}
	ImGui::End();
	// ★ ウェーブ情報の表示
	if (ImGui::Begin("Wave Info")) {
		ImGui::Text("Current Wave: %d", enemyManager->GetCurrentWave());
		ImGui::Text("Alive Enemies: %d", enemyManager->GetAliveEnemyCount());
		ImGui::Text("Total Killed: %d", enemyManager->GetTotalEnemiesKilled());
		ImGui::ProgressBar(enemyManager->GetWaveProgress(), ImVec2(0.0f, 0.0f));

		const char* stateNames[] = {"Waiting", "Spawning", "Active", "Complete"};
		int stateIndex = static_cast<int>(enemyManager->GetWaveState());
		ImGui::Text("Wave State: %s", stateNames[stateIndex]);

		float waveDelay = 3.0f;
		if (ImGui::DragFloat("Wave Delay", &waveDelay, 0.1f, 0.5f, 10.0f)) {
			enemyManager->SetWaveDelay(waveDelay);
		}
		ImGui::End();
	}

#endif // USE_IMGUI
}
void GameScene::Update() {
	GameTimer::GetInstance()->Update();
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}
	if (!isLevelSelecting && !isTransitionIn && !isTransitionOut) {
		bool togglePause = GameBase::GetInstance()->TriggerKey(DIK_ESCAPE) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonStart);
		if (togglePause) {
			isPause = !isPause;
		}
	}

	pause->Update(isPause);
	Pause::Action pauseAction = pause->ConsumeAction();
	if (pauseAction == Pause::Action::kResume) {
		isPause = false;
	} else if (pauseAction == Pause::Action::kTitle) {
		if (!isTransitionOut) {
			nextSceneName = "Title";
			sceneTransition->Initialize(true);
			isTransitionOut = true;
			isPause = false;
		}
		return;
	}

	if (isPause && !isTransitionOut) {
		return;
	}

	// ★ レベルアップ選択中は操作受付
	if (isLevelSelecting) {

		// A で左、D で右
		if (GameBase::GetInstance()->TriggerKey(DIK_A)) {
			cursorIndex = 0;
		}
		if (GameBase::GetInstance()->TriggerKey(DIK_D)) {
			cursorIndex = 1;
		}

		// SPACEで決定
		if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {

			int choice = selectChoices[cursorIndex];
			auto params = player->GetParameters();

			switch (choice) {
			case 0:
				params.AttuckUp++;
				break;
			case 1:
				params.SpeedUp++;
				break;
			case 2:
				params.HPUp++;
				break;
			case 3:
				params.AllowUp++;
				break;
			}

			player->SetParameters(params);

			player->IsLevelUp(false);
			isLevelSelecting = false;
		}

		// 選択中はその他の更新停止
		return;
	}

	DebugImGui();
	
	if (player->GetIsSkillAttack()) {
		pointLights_[1].intensity = 1.0f;
		pointLights_[1].position = {player->GetSkillPosition().x, player->GetSkillPosition().y + 4, player->GetSkillPosition().z};
		
	} else {
		pointLights_[1].intensity = 0.0f;
		pointLights_[1].position = {player->GetPosition().x, player->GetPosition().y + 4, player->GetPosition().z};
	}
	pointLights_[2].position = {player->GetPosition().x, player->GetPosition().y + 2, player->GetPosition().z};

	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);

	skyDome->SetCamera(cameraController->GetCamera());
	player->SetCamera(cameraController->GetCamera());
	field->SetCamera(cameraController->GetCamera());

	ParticleManager::GetInstance()->Update(cameraController->GetCamera());
	skyDome->Update();
	field->Update();
	house->Update(cameraController->GetCamera());
	player->Update();
	// ===========================
	// ★ レベルアップを検知して選択画面へ
	// ===========================
	if (player->GetLv() && !isLevelSelecting) {

		isLevelSelecting = true;

		// ランダムで 0～3 から2個選ぶ
		int a = rand() % 4;
		int b = rand() % 4;
		while (b == a) {
			b = rand() % 4;
		}

		selectChoices[0] = a;
		selectChoices[1] = b;

		cursorIndex = 0; // 左から開始

		// プレイヤーの動きを停止させたい場合はここで何もしない(GameScene が制御)
	}

	// ★ ウェーブシステムの更新
	enemyManager->Update(cameraController->GetCamera(), house->GetPosition(), player->GetPosition(), player->GetIsAlive());
	expCubeManager->Update(cameraController->GetCamera());
	int currentWave = enemyManager->GetCurrentWave();
	if (currentWave != lastWave_) {
		lastWave_ = currentWave;
		currentPhaseSpriteIndex_ = std::clamp(currentWave, 1, 5) - 1;
		isPhaseSpriteActive_ = true;
		isPhaseSpritePaused_ = false;
		phaseSpriteStopTimer_ = 0.0f;
		phaseSpriteX_ = 1280.0f + phaseSpriteSize_.x;
	}

	if (isPhaseSpriteActive_) {
		const float deltaTime = 1.0f / 60.0f;
		const float moveSpeed = 500.0f;
		const float stopDuration = 0.8f;
		const float centerX = 640.0f - phaseSpriteSize_.x / 2.0f;

		if (isPhaseSpritePaused_) {
			phaseSpriteStopTimer_ += deltaTime;
			if (phaseSpriteStopTimer_ >= stopDuration) {
				isPhaseSpritePaused_ = false;
			}
		} else {
			phaseSpriteX_ -= moveSpeed * deltaTime;
			if (phaseSpriteX_ <= centerX) {
				phaseSpriteX_ = centerX;
				isPhaseSpritePaused_ = true;
				phaseSpriteStopTimer_ = 0.0f;
			}
		}

		if (!isPhaseSpritePaused_ && phaseSpriteX_ <= -phaseSpriteSize_.x) {
			isPhaseSpriteActive_ = false;
		}

		auto* phaseSprite = phaseSprites_[currentPhaseSpriteIndex_].get();
		phaseSprite->SetPosition({phaseSpriteX_, phaseSpriteY_});
		phaseSprite->Update();
	}

	// ★ ウェーブクリア判定でゴールを有効化
	// 全ウェーブクリア後にゴールを出すなど、条件は自由に変更可能
	if (enemyManager->GetCurrentWave() >= 5 && enemyManager->IsWaveComplete()) {
		goalActive = true;
	}
	if (goalActive && player->GetIsAlive() && !isTransitionOut) {
		nextSceneName = "Result";
		sceneTransition->Initialize(true);
		isTransitionOut = true;
	}

#ifdef _DEBUG
	if (GameBase::GetInstance()->TriggerKey(DIK_P)) {
		nextSceneName = "Result";
		sceneTransition->Initialize(true);
		isTransitionOut = true;
	}
#endif // _DEBUG

	if (!player->GetIsAlive()) {
		if (!isTransitionOut) {
			nextSceneName = "GameOver";
			sceneTransition->Initialize(true);
			isTransitionOut = true;
		}
	}

	// ===== プレイヤーと敵の当たり判定 =====
	collisionManager_.HandleGameSceneCollisions(*player, *enemyManager, *expCubeManager, *house);

	uimanager->SetPlayerParameters(player->GetParameters());
	uimanager->SetPlayerHP(player->GetHP());

	uimanager->Update();

	particles->SetCameraPos(cameraController->GetCamera()->GetTranslate());
	particles->SetPlayerPos(player->GetPosition());
	particles->Update();

	cameraController->SetPlayerPos(player->GetPosition());

	cameraController->Update();

	if (isTransitionIn || isTransitionOut) {
		sceneTransition->Update();
		if (sceneTransition->IsEnd() && isTransitionIn) {
			isTransitionIn = false;
		}
		if (sceneTransition->IsEnd() && isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene(nextSceneName);
		}
	}
}

void GameScene::Draw() {

	Object3dCommon::GetInstance()->DrawCommon();
	skyDome->Draw();
	field->Draw();
	house->Draw();
	player->Draw();
	enemyManager->Draw();
	expCubeManager->Draw();

	particles->Draw();

	SpriteCommon::GetInstance()->DrawCommon();
	uimanager->Draw();
	if (isPhaseSpriteActive_) {

		phaseSprites_[currentPhaseSpriteIndex_]->Draw();
	}
	// =============================
	// ★ レベルアップ選択画面描画(別スプライト使用版)
	// =============================
	if (isLevelSelecting) {

		// 左と右の選択肢
		int leftID = selectChoices[0];
		int rightID = selectChoices[1];

		// 表示位置(自由に調整可)
		Vector2 leftPos = {350, 300};
		Vector2 rightPos = {750, 300};

		// 左アイコン描画
		levelupIcons[leftID]->SetPosition(leftPos);
		levelupIcons[leftID]->SetColor(cursorIndex == 0 ? Vector4(1, 1, 0, 1) : Vector4(1, 1, 1, 1));
		levelupIcons[leftID]->Update();
		levelupIcons[leftID]->Draw();

		// 右アイコン描画
		levelupIcons[rightID]->SetPosition(rightPos);
		levelupIcons[rightID]->SetColor(cursorIndex == 1 ? Vector4(1, 1, 0, 1) : Vector4(1, 1, 1, 1));
		levelupIcons[rightID]->Update();
		levelupIcons[rightID]->Draw();
	}
	pause->Draw();
	if (isTransitionIn || isTransitionOut) {
		sceneTransition->Draw();
	}
}