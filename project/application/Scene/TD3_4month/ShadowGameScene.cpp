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

ShadowGameScene::ShadowGameScene()
{
    //BGMの管理
    BGMManager::Load();

    //シーン遷移の設定
    transition_ = std::make_unique<SceneTransition>();

    //プレイヤーの生成
    player_ = std::make_unique<Player>();
    //カメラコントローラー
    cameraController_ = CameraController::GetInstance();
    cameraController_->SetPlayer(player_.get());

   
    // エレベーター
    elevator_ = std::make_unique<Elevator>();
    //セーブポイント紳士
    gentleman_ = std::make_unique<Gentleman>();
    //エレベータールーム
    elevatorRoomManager_ = std::make_unique<ElevatorRoomManager>();

	// UI管理
	uiManager_ = std::make_unique<UIManager>();
    //PlayerCameraをセットする
    SetPlayerCamera(cameraController_->GetPlayerCamera());

    damageOverlay_ = std::make_unique<DamageOverlay>();
	stageManager_ = std::make_unique<StageManager>();

}

ShadowGameScene::~ShadowGameScene()
{
    //BGMの管理
    BGMManager::UnLoad();
}

void ShadowGameScene::Initialize()
{


    BGMManager::Initialize();

    uiManager_->Initialize();
    damageOverlay_->Initialize();

    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;
    nextSceneName_.clear();

	// プレイヤーの初期化
	player_->Initialize();
	stageManager_->SetPlayer(player_.get());
	stageManager_->SetElevatorManager(elevatorRoomManager_.get());
    PlayerCommand::Initialize();

	// カメラコントローラー
	cameraController_->Initialize();
	stageManager_->SetPlayerCamera(cameraController_->GetPlayerCamera());


    InitializeLights();


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

    collisionManager_ = std::make_unique<CollisionManager>();
	stageManager_->SetCollisionManager(collisionManager_.get());

	stageManager_->ChangeStage("MirrorStage");
    Update();


}

void ShadowGameScene::Update() {


	// カメラの更新処理
	UpdateCamera();
	// ライトの更新処理
	UpdateLight();

	if (!stageManager_->IsCurrentEventRunning()) {
		// UI管理
		uiManager_->Update();
		PlayerCommand::SetIsUiInputLocked(UIManager::GetIsPause());
		// シーン遷移の更新処理
		UpdateSceneTransition();
	}

	// ゲームオブジェクトの更新処理
	UpdateGameObject();
	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	damageOverlay_->Update(deltaTime, stageManager_->GetPlayerHp(), stageManager_->GetPlayerMaxHp());
	if (stageManager_->DidPlayerTakeDamage()) {
		damageOverlay_->StartDisplay();
	}
	if (stageManager_->IsPlayerDead() && !isTransitionOut_) {
		transition_->Initialize(true);
		isTransitionOut_ = true;
		nextSceneName_ = "GameOver";
	}
	stageManager_->Update();
	// オブジェクトの当たり判定
	CheckCollision();
	
}

void ShadowGameScene::Draw() {
	auto* object3dCommon = Object3dCommon::GetInstance();
	const bool shadowFlags[4] = {useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_};
	for (int i = 0; i < 4; ++i) {
		if (!shadowFlags[i]) {
			continue;
		}
		object3dCommon->SetShadowMapEnabled(i == 0, i == 1, i == 2, i == 3);
		object3dCommon->BeginShadowMapPass();
		object3dCommon->DrawCommonShadow();
		stageManager_->ShadowMapDraw();
		object3dCommon->EndShadowMapPass();
	}
	object3dCommon->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
	// ゲームオブジェクトの描画処理
	DrawModel();

	// スプライト共通
	SpriteCommon::GetInstance()->DrawCommon();
	damageOverlay_->Draw();
	if (!stageManager_->IsCurrentEventRunning()) {
		// UI管理を描画する
		uiManager_->Draw();
	}

	// シーン遷移の描画処理
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

	collisionManager_->AddCollider(player_.get());
	for (auto& wall : elevatorRoomManager_->GetWalls()) {
		collisionManager_->AddCollider(wall.get());
	}
	for (auto& system : elevator_->GetAutoLockSys()) {
		collisionManager_->AddCollider(system.get());
	}
	stageManager_->CheckCollision();
	collisionManager_->CheckAllCollisions();
}


void ShadowGameScene::InitializeLights() {

	directionalLight_.color = {1.0f, 1.0f, 0.75f, 1.0f};
	directionalLight_.direction = {0.0f, 1.0f, 0.0f};
	directionalLight_.intensity = 0.25f;
	stageManager_->SetDirectionalShadowEnabled(useDirectionalShadow_);
}
#pragma region //private更新処理
void ShadowGameScene::UpdateCamera() {

	cameraController_->Update();
	Camera* sceneCamera = cameraController_->GetPlayerCamera()->GetCamera();
	SetSceneCameraForDraw(sceneCamera);
	Object3dCommon::GetInstance()->SetDefaultCamera(sceneCamera);
}

void ShadowGameScene::UpdateSceneTransition() {
    //if (/*door_->GetOpenMassage() &&! */isTransitionOut_) {
    //    transition_->Initialize(true);
    //    isTransitionOut_ = true;
    //    nextSceneName_ = "Result";
    //}

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

void ShadowGameScene::UpdateGameObject() {
	// プレイヤー
	player_->Update();

	// エレベーター
	elevator_->Update();
	// エレベータールーム管理
	elevatorRoomManager_->Update();
	// セーブポイント紳士
	gentleman_->Update();

	ParticleManager::GetInstance()->Update(cameraController_->GetPlayerCamera()->GetCamera());
}
void ShadowGameScene::UpdateLight() {
#pragma region // Lightを組み込む

	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);

#pragma endregion

#ifdef USE_IMGUI
	if (ImGui::TreeNode("DirectionalLight")) {
		ImGui::Checkbox("DirectionalShadow", &useDirectionalShadow_);
		ImGui::TreePop();
	}
#endif
	stageManager_->SetDirectionalShadowEnabled(useDirectionalShadow_);
}
#pragma endregion

#pragma region // private描画処理
void ShadowGameScene::DrawSceneTransition() {
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Draw();
    }
}

void ShadowGameScene::DrawModel() {
	Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
	Object3dCommon::GetInstance()->DrawCommon();
	stageManager_->MainDraw();
	SetCameraAndDraw(cameraController_->GetPlayerCamera()->GetCamera(), true, true, false);
}
void ShadowGameScene::DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle, bool drawPlayer)
{
    //エレベータルーム
    elevatorRoomManager_->Draw();

    //エレベーター    
    elevator_->Draw();
    //セーブポイント紳士
    gentleman_->Draw();

    if (!isShadow) {
        Object3dCommon::GetInstance()->DrawCommonSkinning();
    }

    if (drawPlayer) {
        // プレイヤーの描画処理
        player_->Draw();
    }


}

void ShadowGameScene::SetSceneCameraForDraw(Camera* camera) {
    player_->SetCamera(camera);
    elevatorRoomManager_->SetCamera(camera);

    elevator_->SetCamera(camera);
    gentleman_->SetCamera(camera);
}
void ShadowGameScene::SetPlayerCamera(PlayerCamera* playerCamera)
{
    gentleman_->SetPlayerCamera(playerCamera);
    elevator_->SetPlayerCamera(playerCamera);
}
void ShadowGameScene::SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle, bool drawPlayer)
{
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);
    SetSceneCameraForDraw(camera);
    Object3dCommon::GetInstance()->DrawCommon();
    DrawGameObject(false, drawPortal, isDrawParticle, drawPlayer);
}
#pragma endregion