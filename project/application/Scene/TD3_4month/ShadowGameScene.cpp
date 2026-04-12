#define NOMINMAX
#include "ShadowGameScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include<numbers>
#include"RigidBody.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include "Particle/ParticleManager.h"
#include"GameObject/BGMManager/BGMManager.h"
#include "GameObject/SEManager/SEManager.h"
#include <algorithm>
#include <cmath>
#include"GameSave/GameSave.h"

ShadowGameScene::ShadowGameScene() {
	// BGMの管理
	BGMManager::Load();

	// シーン遷移の設定
	transition_ = std::make_unique<SceneTransition>();

	// プレイヤーの生成
	player_ = std::make_unique<Player>();
	auto& gameSave = GameSave::GetInstance();

	if (gameSave.GetInitStart()) {
		gameSave.InitData();
	} else {
		// 一旦ここでロード
		gameSave.Load(gameSave.GetSelectSlotIndex());
	}
	progressSaveData_ = gameSave.GetProgressSaveData();
	// プレイヤーの初期化
	player_->Initialize();
	player_->SetTransform(gameSave.GetPlayerSaveData().transform);
	// カメラコントローラー
	cameraController_ = CameraController::GetInstance();
	cameraController_->SetPlayer(player_.get());
	stageManager_ = std::make_unique<StageManager>(player_.get());
	stageManager_->CreateStage("MirrorStage");
	// エレベーター
	elevator_ = std::make_unique<Elevator>();
	// セーブポイント紳士
	gentleman_ = std::make_unique<Gentleman>();

	// エレベータールーム
	elevatorRoomManager_ = std::make_unique<ElevatorRoomManager>();
	// 衝突管理
	collisionManager_ = stageManager_->GetCollisionManager();
	// UI管理
	uiManager_ = std::make_unique<UIManager>();
	GentlemanMenu::SetPlayerCamera(cameraController_->GetPlayerCamera());
	GentlemanMenu::SetPlayerTransform(&player_->GetTransform());
	GentlemanMenu::SetProgressSaveData(&progressSaveData_);
	// 最初のイベント
	firstEvent_ = std::make_unique<FirstGameEvent>();
	// PlayerCameraをセットする
	SetPlayerCamera(cameraController_->GetPlayerCamera());

	damageOverlay_ = std::make_unique<DamageOverlay>();
}

ShadowGameScene::~ShadowGameScene()
{
    //BGMの管理
    BGMManager::UnLoad();
}

void ShadowGameScene::Initialize()
{
    uiManager_->Initialize();


    BGMManager::Initialize();

    damageOverlay_->Initialize();

    noiseTimer_ = kNoiseTimer_;
    isNoise_ = false;


    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;
    nextSceneName_.clear();


		auto& gameSave = GameSave::GetInstance();

	if (gameSave.GetInitStart()) {
		gameSave.InitData();
	} else {
		// 一旦ここでロード
		gameSave.Load(gameSave.GetSelectSlotIndex());
	}
	progressSaveData_ = gameSave.GetProgressSaveData();

    PlayerCommand::Initialize();

    //カメラコントローラー
    cameraController_->Initialize();
    cameraController_->GetInstance()->GetPlayerCamera()->SetParam(gameSave.GetCameraSaveData());

 
    InitializeLights();
	stageManager_->SetPlayerCamera(cameraController_->GetPlayerCamera());
	stageManager_->InitializeStage();
    // エレベーター
    elevator_->Initialize();
    //セーブポイント紳士
    gentleman_->Initialize();
    //エレベータールーム
    elevatorRoomManager_->Initialize();

    //カーソルを画面中央に設定する
    uiManager_->CursorHideAndStop();
    //カメラをセットする
    SetSceneCameraForDraw(cameraController_->GetPlayerCamera()->GetCamera());

    //最初のイベントをセットする
    currentEvent_ = firstEvent_.get();


    Update();

    currentEvent_->StartEvent();

}

void ShadowGameScene::Update() {

	currentEvent_->Update();

	// ライトの更新処理
	UpdateLight();
	// ポストエフェクトの更新処理
	UpdatePostEffect();

	if (!currentEvent_->IsRunning()) {
		// UI管理
		uiManager_->Update();
		PlayerCommand::SetIsUiInputLocked(UIManager::GetIsPause());
		// シーン遷移の更新処理
		UpdateSceneTransition();
	}

	// ゲームオブジェクトの更新処理
	UpdateGameObject();
	// カメラの更新処理
	UpdateCamera();
	// ポータル管理 カメラの更新後に行う
	stageManager_->UpdatePortal();
	UpdatePlayerDamage();
	// オブジェクトの当たり判定
	CheckCollision();
}

void ShadowGameScene::Draw()
{
    //ゲームオブジェクトの描画処理
    DrawModel();

    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
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

void ShadowGameScene::DebugImGui()
{
#ifdef USE_IMGUI
    ImGui::Begin("shadowGameScene");
    ImGui::End();
#endif // USE_IMGUI
}

void ShadowGameScene::CheckCollision() {
	if (!collisionManager_) {
		return;
	}

	collisionManager_->ClearColliders();
	stageManager_->CheckCollision(collisionManager_.get());
	if (stageManager_->GetCollisionManager()) {
	collisionManager_ = stageManager_->GetCollisionManager();
	}
	collisionManager_->AddCollider(player_.get());

	for (auto& wall : elevatorRoomManager_->GetWalls()) {
		collisionManager_->AddCollider(wall.get());
	}

	for (auto& system : elevator_->GetAutoLockSys()) {
		collisionManager_->AddCollider(system.get());
	}

	collisionManager_->CheckAllCollisions();
}

void ShadowGameScene::InitializeLights() {
	directionalLight_.color = {1.0f, 1.0f, 0.75f, 1.0f};
	directionalLight_.direction = {0.0f, 1.0f, 0.0f};
	directionalLight_.intensity = 0.25f;
}
#pragma region // private更新処理
void ShadowGameScene::UpdateCamera()
{

    cameraController_->Update();

    Object3dCommon::GetInstance()->SetDefaultCamera(cameraController_->GetPlayerCamera()->GetCamera());

}

void ShadowGameScene::UpdateSceneTransition() {
    if (/*door_->GetOpenMassage() &&! */isTransitionOut_) {
        transition_->Initialize(true);
        isTransitionOut_ = true;
        nextSceneName_ = "Result";
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
			if (portal->GetIsPlayerHit()) {
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
	stageManager_->UpdateGameObject(cameraController_->GetPlayerCamera()->GetCamera(), directionalLight_.direction);
	// エレベーター
	elevator_->Update();
	// エレベータールーム管理
	elevatorRoomManager_->Update();
	// セーブポイント紳士
	gentleman_->Update();

	ParticleManager::GetInstance()->Update(cameraController_->GetPlayerCamera()->GetCamera());
}
void ShadowGameScene::UpdatePlayerDamage() {

	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	player_->UpdatePlayerDamage(deltaTime);
	damageOverlay_->Update(deltaTime, player_->GetHP(), player_->GetMaxHP());
}

void ShadowGameScene::UpdateLight() {
#pragma region // Lightを組み込む
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(stageManager_->GetPointLights(), stageManager_->GetActivePointLightCount());
	Object3dCommon::GetInstance()->SetSpotLights(stageManager_->GetSpotLights(), stageManager_->GetActiveSpotLightCount());
	Object3dCommon::GetInstance()->SetAreaLights(stageManager_->GetAreaLights(), stageManager_->GetActiveAreaLightCount());
	Object3dCommon::GetInstance()->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
#pragma endregion

#ifdef USE_IMGUI
	if (ImGui::TreeNode("Light")) {
		AreaCommonLight* areaLights = stageManager_->GetAreaLights();
		PointCommonLight* pointLights = stageManager_->GetPointLights();
		SpotCommonLight* spotLights = stageManager_->GetSpotLights();
		if (areaLights != nullptr) {
			ImGui::DragFloat3("Area0Position", &areaLights[0].position.x, 0.1f);
			ImGui::DragFloat3("Area1Position", &areaLights[1].position.x, 0.1f);
		}
		ImGui::Checkbox("DirectionalShadow", &useDirectionalShadow_);
		ImGui::Checkbox("PointShadow", &usePointShadow_);
		ImGui::Checkbox("SpotShadow", &useSpotShadow_);
		ImGui::Checkbox("AreaShadow", &useAreaShadow_);
		if (pointLights != nullptr) {
			pointLights[0].shadowEnabled = usePointShadow_ ? 1 : 0;
		}
		if (spotLights != nullptr) {
			spotLights[0].shadowEnabled = useSpotShadow_ ? 1 : 0;
		}
		if (areaLights != nullptr) {
			areaLights[0].shadowEnabled = useAreaShadow_ ? 1 : 0;
		}
		ImGui::TreePop();
	}
#endif
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
	const bool shadowFlags[4] = {useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_};
	for (int i = 0; i < 4; ++i) {
		if (!shadowFlags[i]) {
			continue;
		}
		object3dCommon->SetShadowMapEnabled(i == 0, i == 1, i == 2, i == 3);
		object3dCommon->BeginShadowMapPass();
		object3dCommon->DrawCommonShadow();
		DrawGameObject(true, false, false, true, true);
		object3dCommon->EndShadowMapPass();
	}
	object3dCommon->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
	//=======================shadowマップの終了↑=======================

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
	// セーブポイント紳士
	gentleman_->Draw();

	if (drawPlayer) {
		// プレイヤーの描画処理
		player_->Draw();
	}

	if (drawPlayerHead) {
	}
}

void ShadowGameScene::SetSceneCameraForDraw(Camera* camera) {
	player_->SetCamera(camera);
	stageManager_->SetSceneCameraForDraw(camera);
	elevatorRoomManager_->SetCamera(camera);
	elevator_->SetCamera(camera);
	gentleman_->SetCamera(camera);
}
void ShadowGameScene::SetPlayerCamera(PlayerCamera* playerCamera) {
	stageManager_->SetPlayerCamera(playerCamera);
	gentleman_->SetPlayerCamera(playerCamera);
	elevator_->SetPlayerCamera(playerCamera);
}
void ShadowGameScene::SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle, bool drawPlayer, bool drawPlayerHead) {
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	SetSceneCameraForDraw(camera);
	Object3dCommon::GetInstance()->DrawCommon();
	DrawGameObject(false, drawPortal, isDrawParticle, drawPlayer, drawPlayerHead);
}
#pragma endregion