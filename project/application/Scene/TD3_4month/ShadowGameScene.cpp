#define NOMINMAX
#include "ShadowGameScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include "Particle/ParticleManager.h"
#include"GameObject/BGMManager/BGMManager.h"
#include"GameSave/GameSave.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include "GameObject/UI/Inventory/Inventory.h"

ShadowGameScene::ShadowGameScene() {
	// BGMの管理
	BGMManager::Load();

	// シーン遷移の設定
	transition_ = std::make_unique<SceneTransition>();

	// プレイヤーの生成
	player_ = std::make_unique<Player>();
	auto& gameSave = GameSave::GetInstance();

	if (gameSave.GetInitStart()) {
		gameSave.LoadFirstStage();
	} else {
		// 一旦ここでロード
		gameSave.LoadFromIndex(gameSave.GetSelectSlotIndex());
	}

	progressSaveData_ = gameSave.GetProgressSaveData();
	// プレイヤーの初期化
	player_->Initialize();
	player_->SetTransform(gameSave.GetPlayerSaveData().transform);
	// カメラコントローラー
	cameraController_ = CameraController::GetInstance();
	cameraController_->SetPlayer(player_.get());
	
	lightManager_ = std::make_unique<Yoshida::LightManager>();
	
	stageManager_ = std::make_unique<StageManager>(player_.get());
	stageManager_->CreateStage(progressSaveData_.currentStageName);
	// エレベーター
	elevator_ = std::make_unique<Elevator>();
	//紳士管理
	gentleManManager_ = std::make_unique<GentleManManager>();
	gentleManManager_->SetPlayer(player_.get());
	gentleManManager_->SetProgressSaveData(&progressSaveData_);
	// エレベータールーム
	elevatorRoomManager_ = std::make_unique<ElevatorRoomManager>();
	// 衝突管理
	collisionManager_ = std::make_unique<CollisionManager>();
	stageManager_->SetCollisionManager(collisionManager_.get());
	// UI管理
	uiManager_ = std::make_unique<UIManager>();

	// 最初のイベント
	firstEvent_ = std::make_unique<FirstGameEvent>();
	// PlayerCameraをセットする
	SetPlayerCamera(cameraController_->GetPlayerCamera());

	damageOverlay_ = std::make_unique<DamageOverlay>();
	skyBox_ = std::make_unique<SkyBox>();
}

ShadowGameScene::~ShadowGameScene()
{
    //BGMの管理
    BGMManager::UnLoad();
}

void ShadowGameScene::Initialize()
{
	Hierarchy* hierarchy = Hierarchy::GetInstance();
	hierarchy->BeginRegisterFile("ShadowGameScene_objectEditors.json");

    uiManager_->Initialize();
    BGMManager::Initialize();

    damageOverlay_->Initialize();
	skyBox_->Initialize();
	skyBox_->SetDDSTexture("Resources/SkyBox/forest.dds");
	skyBox_->SetCamera(cameraController_->GetPlayerCamera()->GetCamera());

    noiseTimer_ = kNoiseTimer_;
    isNoise_ = false;

    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;
    nextSceneName_.clear();

	auto& gameSave = GameSave::GetInstance();

	if (gameSave.GetInitStart()) {
		gameSave.LoadFirstStage();
	} else {
		// 一旦ここでロード
		gameSave.LoadFromIndex(gameSave.GetSelectSlotIndex());
	}
	progressSaveData_ = gameSave.GetProgressSaveData();

    PlayerCommand::Initialize();
    //カメラコントローラー
    cameraController_->Initialize();
    cameraController_->GetInstance()->GetPlayerCamera()->SetParam(gameSave.GetCameraSaveData());

	lightManager_->Initialize();
	//懐中電灯共通のプログレスセーブデータのポインタを格納
	Flashlight::SetProgressSaveDataPtr(&progressSaveData_);
	//紳士管理
	gentleManManager_->Initialize();

	// エレベータールーム
	elevatorRoomManager_->Initialize();
	// エレベーター
	elevator_->Initialize();
	hierarchy->LoadObjectEditorsFromJsonIfExists("ShadowGameScene_objectEditors.json");
	hierarchy->EndRegisterFile();
	stageManager_->SetPlayerCamera(cameraController_->GetPlayerCamera());
	stageManager_->SetLightManager(lightManager_.get());
	stageManager_->InitializeStage();
	lightManager_->SetPointLight(cameraController_->GetPlayerCamera()->GetPointLight(),0);


    //カーソルを画面中央に設定する
    uiManager_->CursorHideAndStop();
    //カメラをセットする
    SetSceneCameraForDraw(cameraController_->GetPlayerCamera()->GetCamera());

    //最初のイベントをセットする
    currentEvent_ = firstEvent_.get();



    Update();

    currentEvent_->StartEvent();
	uiManager_->ShowKeyLostAtStageStartMessage();
}

void ShadowGameScene::Update() {
#ifdef USE_IMGUI
	DebugImGui();
#endif
	// BGMの更新処理
	BGMManager::Update();

	StageTransition();
	currentEvent_->Update();

	// ライトの更新処理
	UpdateLight();
	// ポストエフェクトの更新処理
	UpdatePostEffect();

	if (!currentEvent_->IsRunning()) {
		// UI管理
		uiManager_->Update();
		if (Inventory::GetInstance()->ConsumeItemUseEvent()) {
			player_->SetHP(player_->GetMaxHP());
		}
		PlayerCommand::SetIsUiInputLocked(UIManager::IsUiOperationBlocked());
	}
	// シーン遷移の更新処理
	UpdateSceneTransition();

	// ゲームオブジェクトの更新処理
	UpdateGameObject();
	// カメラの更新処理
	UpdateCamera();
	// ポータル管理 カメラの更新後に行う
	stageManager_->UpdatePortal();
	UpdatePlayerDamage();
	// オブジェクトの当たり判定
	CheckCollision();
	if (transition_->IsEnd() && isTransitionOut_) {
		if (!nextSceneName_.empty()) {
			// シーンの切り替え
			SceneManager::GetInstance()->ChangeScene(nextSceneName_);
		}
	}
}

void ShadowGameScene::Draw()
{
    //ゲームオブジェクトの描画処理
    DrawModel();

    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
	//ステージマネージャーでスプライトを描画
	stageManager_->DrawSprite();
	damageOverlay_->Draw();
    if (!currentEvent_->IsRunning()) {
        //UI管理を描画する
        uiManager_->Draw();
    }

    //シーン遷移の描画処理
    DrawSceneTransition();
}

void ShadowGameScene::Finalize()
{
    BGMManager::UnLoad();
}

void ShadowGameScene::DebugImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("shadowGameScene");
	static constexpr const char* kStageNames[] = {"MirrorStage", "LightStage", "TutorialStage", "RadiconStage","GentleManStage","RestroomStage"};
	int stageIndex = 0;
	if (progressSaveData_.currentStageName == "LightStage") {
		stageIndex = 1;
	} else if (progressSaveData_.currentStageName == "TutorialStage") {
		stageIndex = 2;
	} else if (progressSaveData_.currentStageName == "RadiconStage") {
		stageIndex = 3;
	} else if (progressSaveData_.currentStageName == "GentleManStage") {
		stageIndex = 4;
	} else if (progressSaveData_.currentStageName == "RestroomStage") {
		stageIndex = 5;
	}

	if (ImGui::Combo("Stage", &stageIndex, kStageNames, IM_ARRAYSIZE(kStageNames))) {
		ChangeStage(kStageNames[stageIndex]);
	}

	ImGui::Checkbox("isGameClear", &progressSaveData_.isGameClear);
	ImGui::Checkbox("isKeyHave", &progressSaveData_.isKeyHave);
	ImGui::Checkbox("isLightHave", &progressSaveData_.isLightHave);

	ImGui::End();
#endif // USE_IMGUI
}

void ShadowGameScene::ChangeStage(const std::string& stageName) {
	if (stageName == progressSaveData_.currentStageName) {
		return;
	}
	progressSaveData_.currentStageName = stageName;
	stageManager_->CreateStage(progressSaveData_.currentStageName);
	stageManager_->SetPlayerCamera(cameraController_->GetPlayerCamera());
	stageManager_->SetLightManager(lightManager_.get());
	stageManager_->SetPlayer(player_.get());
	stageManager_->SetCollisionManager(collisionManager_.get());
	stageManager_->InitializeStage();
	SetSceneCameraForDraw(cameraController_->GetPlayerCamera()->GetCamera());

	uiManager_->ShowKeyLostAtStageStartMessage();
}

void ShadowGameScene::CheckCollision() {
	if (!collisionManager_) {
		return;
	}

	collisionManager_->ClearColliders();

	collisionManager_->AddCollider(player_.get());

	for (auto&[name, wall]:elevatorRoomManager_->GetColliders()) {
		collisionManager_->AddCollider(wall.get());
	}

	for (auto& system : elevator_->GetAutoLockSys()) {
		collisionManager_->AddCollider(system.get());
	}
	for (auto& [name,collider] : elevator_->GetColliders()) {
		collisionManager_->AddCollider(collider.get());
	}


	collisionManager_->AddCollider(gentleManManager_->GetGentleman());

	stageManager_->SetCollisionManager(collisionManager_.get());
	stageManager_->CheckCollision();
}


#pragma region // private更新処理
void ShadowGameScene::UpdateCamera()
{

    cameraController_->Update();

    Object3dCommon::GetInstance()->SetDefaultCamera(cameraController_->GetPlayerCamera()->GetCamera());

}

void ShadowGameScene::UpdateSceneTransition() {


	if (Key::IsGetKey()) {
		if (progressSaveData_.currentStageName == "TutorialStage" && !isTransitionOut_) {
			transition_->Initialize(true);
			isTransitionIn_ = false;
			isTransitionOut_ = true;
			nextSceneName_ = "Result";
		}
	}

    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Update();
        if (transition_->IsEnd() && isTransitionIn_) {
            isTransitionIn_ = false;
        }
        if (transition_->IsEnd() && isTransitionOut_) {
            if (!nextSceneName_.empty()) {
                // シーンの切り替え
                SceneManager::GetInstance()->ChangeScene(nextSceneName_);
            }
        }

    }
}

void ShadowGameScene::UpdatePostEffect() {
	bool vignetteStrength = true;

	Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(false);
	Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(false);
	Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength);
	Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength);

	if (auto* portalManager = stageManager_->GetPortalManager()) {
		for (auto& portal : portalManager->GetPortals()) {
			if (portal->GetIsPlayerCanWarp()) {
				if (!isNoise_) {
					isNoise_ = true;
				}
			}
		}
	}

	if (isNoise_) {
		float randomNoiseScale = 1.0f;
		noiseTimer_ -= YoshidaMath::kDeltaTime;
		if (noiseTimer_ <= 0.0f) {
			isNoise_ = false;
			noiseTimer_ = kNoiseTimer_;
		}
	}
	BlendMode randomNoiseBlendMode = kBlendModeSub;

	Object3dCommon::GetInstance()->SetRandomNoiseEnabled(isNoise_);
	Object3dCommon::GetInstance()->SetRandomNoiseScale(noiseTimer_);
	Object3dCommon::GetInstance()->SetRandomNoiseBlendMode(randomNoiseBlendMode);
}

void ShadowGameScene::UpdateGameObject() {
	player_->Update();
	stageManager_->SetPlayer(player_.get());
	stageManager_->UpdateGameObject(cameraController_->GetPlayerCamera()->GetCamera(),lightManager_->GetDirectionalLight().direction);
	// エレベーター
	elevator_->Update();
	// エレベータールーム管理
	elevatorRoomManager_->Update();
	//紳士管理
	gentleManManager_->Update();
	ParticleManager::GetInstance()->Update(cameraController_->GetPlayerCamera()->GetCamera());
}
void ShadowGameScene::UpdatePlayerDamage() {
	static constexpr float kCoffeeDamageAmount = 1.0f;
	static constexpr float kCoffeeHitRadius = 0.45f;
	static constexpr float kCoffeeHitMinSpeed = 1.75f;

	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	player_->UpdatePlayerDamage(deltaTime);
	if (player_->GetDamageCoolDownTimer() <= 0.0f) {
		const bool isHitByStageHazard = stageManager_->CheckHitPlayerByStageHazard(player_->GetTransform().translate, kCoffeeHitRadius, kCoffeeHitMinSpeed);
		if (isHitByStageHazard) {
			player_->ApplyPlayerDamage(kCoffeeDamageAmount);
		}
	}
	damageOverlay_->Update(deltaTime, player_->GetHP(), player_->GetMaxHP());
	if (player_->GetHP() <= 0) {
		if (!isTransitionOut_) {
			transition_->Initialize(true);
			isTransitionOut_ = true;
			nextSceneName_ = "GameOver";
		}
	}
}

void ShadowGameScene::UpdateLight() {
	lightManager_->Update();
	lightManager_->SetPointLight(cameraController_->GetPlayerCamera()->GetPointLight(), 0);

}
void ShadowGameScene::StageTransition()
{
	if (Key::IsGetKey() && Door::GetOpenMassage()) {
		
		if (!progressSaveData_.isGameClear) {
			progressSaveData_.isGameClear = true;
		}

	
	}



	//ステージの切り替え
	if (elevator_->IsSceneTransitionStart()&& progressSaveData_.isGameClear) {

		if (progressSaveData_.currentStageName == "MirrorStage") {
			ChangeStage("TutorialStage");
			progressSaveData_.isGameClear = false;
		} 
	
	}
}
#pragma endregion
#pragma region // private描画処理
void ShadowGameScene::DrawSceneTransition() {
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Draw();
    }
}

void ShadowGameScene::DrawModel() {
	//=======================shadowマップの開始↓=======================
	auto* object3dCommon = Object3dCommon::GetInstance();
	// レイトレースシャドウに移行したため、シャドウマップパスは無効化
	object3dCommon->SetShadowMapEnabled(false, false, false, false);

	if (auto* portalManager = stageManager_->GetPortalManager()) {
		for (auto& portal : portalManager->GetPortals()) {
			portal->BeginRender();
			auto* portalCamera = portal->GetCamera();
			SetCameraAndDraw(portalCamera, false, false, true, true);
			portal->TransitionToShaderResource();
		}
	}

	Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
	SetCameraAndDraw(cameraController_->GetPlayerCamera()->GetCamera(), true, true, true, false);
}
void ShadowGameScene::DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle, bool drawPlayer, bool drawPlayerHead) {
	// エレベータルーム
	elevatorRoomManager_->Draw();
	stageManager_->DrawModel(isShadow, drawPortal, isDrawParticle);

	if (!isShadow) {
		Object3dCommon::GetInstance()->DrawCommonSkinning();
	}

	// エレベーター
	elevator_->Draw();
	gentleManManager_->Draw();

	if (drawPlayer) {
		// プレイヤーの描画処理
		player_->Draw();
	}

	if (drawPlayerHead) {
	}
}

void ShadowGameScene::SetSceneCameraForDraw(Camera* camera) {
	skyBox_->SetCamera(camera);
	player_->SetCamera(camera);
	stageManager_->SetSceneCameraForDraw(camera);
	elevatorRoomManager_->SetCamera(camera);
	elevator_->SetCamera(camera);
	gentleManManager_->SetCamera(camera);
}
void ShadowGameScene::SetPlayerCamera(PlayerCamera* playerCamera) {
	stageManager_->SetPlayerCamera(playerCamera);

	gentleManManager_->SetPlayerCamera(playerCamera);
	elevator_->SetPlayerCamera(playerCamera);
}
void ShadowGameScene::SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle, bool drawPlayer, bool drawPlayerHead) {
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	SetSceneCameraForDraw(camera);
	auto* object3dCommon = Object3dCommon::GetInstance();
	object3dCommon->DrawCommonSkybox();
	skyBox_->Update();
	skyBox_->Draw();
	object3dCommon->DrawCommon();
	DrawGameObject(false, drawPortal, isDrawParticle, drawPlayer, drawPlayerHead);
}
#pragma endregion
