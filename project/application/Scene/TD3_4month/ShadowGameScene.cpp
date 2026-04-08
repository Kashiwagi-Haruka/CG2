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

    noiseTimer_ = kNoiseTimer_;
    isNoise_ = false;
    playerHp_ = kPlayerMaxHp_;
    damageCooldownTimer_ = 0.0f;

    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;
    nextSceneName_.clear();

    //プレイヤーの初期化
    player_->Initialize();
    PlayerCommand::Initialize();

    //カメラコントローラー
    cameraController_->Initialize();


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



	stageManager_->ChangeStage("MirrorStage");
    Update();


}

void ShadowGameScene::Update()
{
	stageManager_->Update();
 
    //カメラの更新処理
    UpdateCamera();
    //ライトの更新処理
    UpdateLight();
    //ポストエフェクトの更新処理
    UpdatePostEffect();

    if (!currentEvent_->IsRunning()) {
        //UI管理
        uiManager_->Update();
        PlayerCommand::SetIsUiInputLocked(UIManager::GetIsPause());
        //シーン遷移の更新処理
        UpdateSceneTransition();
    }

    //ゲームオブジェクトの更新処理
    UpdateGameObject();
    UpdatePlayerDamage();
    //オブジェクトの当たり判定
    CheckCollision();


}

void ShadowGameScene::Draw()
{
	stageManager_->Draw();
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

void ShadowGameScene::CheckCollision()
{
	collisionManager_->AddCollider(player_.get());
    for (auto& wall : elevatorRoomManager_->GetWalls()) {
        collisionManager_->AddCollider(wall.get());
    }
	for (auto& system : elevator_->GetAutoLockSys()) {
		collisionManager_->AddCollider(system.get());
	}

    collisionManager_->CheckAllCollisions();
}

void ShadowGameScene::InitializeLights()
{



    directionalLight_.color = { 1.0f, 1.0f, 0.75f, 1.0f };
    directionalLight_.direction = { 0.0f, 1.0f, 0.0f };
    directionalLight_.intensity = 0.25f;

 

}
#pragma region //private更新処理
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

void ShadowGameScene::UpdatePostEffect()
{
    bool vignetteStrength = true;

    Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(false);
    Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(false);
    Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength);
    Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength);

    for (auto& portal : portalManager_->GetPortals()) {
        if (portal->GetIsPlayerHit()) {
            if (!isNoise_) {
                isNoise_ = true;
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
void ShadowGameScene::UpdatePlayerDamage() {
    const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
    damageCooldownTimer_ = std::max(0.0f, damageCooldownTimer_ - deltaTime);

    constexpr float kHpRegenPerSecond = 0.2f;
    playerHp_ = std::min(kPlayerMaxHp_, playerHp_ + (kHpRegenPerSecond * deltaTime));

    damageOverlay_->Update(deltaTime, playerHp_, kPlayerMaxHp_);

    constexpr float kCoffeeHitSpeedThreshold = 2.0f;
    constexpr float kPlayerHitRadius = 0.45f;
    constexpr float kDamageCooldown = 0.7f;

    if (damageCooldownTimer_ <= 0.0f && coffees_->CheckHitPlayer(player_->GetWorldPosition(), kPlayerHitRadius, kCoffeeHitSpeedThreshold)) {
        ApplyPlayerDamage(1.0f);
        damageCooldownTimer_ = kDamageCooldown;
        damageOverlay_->StartDisplay();

        if (playerHp_ <= 0.0f) {
            if (!isTransitionOut_) {
                transition_->Initialize(true);
                isTransitionOut_ = true;
                nextSceneName_ = "GameOver";
            }
            return;
        }
    }
}

void ShadowGameScene::ApplyPlayerDamage(float damageAmount) {
    const float prevHp = playerHp_;
    playerHp_ = std::max(0.0f, playerHp_ - damageAmount);
    if (playerHp_ < prevHp) {
        SEManager::SoundPlay(SEManager::DAMAGE);
    }
    if (playerHp_ > 1.0f && playerHp_ < kPlayerMaxHp_) {
        playerHp_ = std::floor(playerHp_);
    }
}
void ShadowGameScene::UpdateLight() {
#pragma region // Lightを組み込む

    spotLights_[0] = flashlight_->GetSpotLight();

    pointLights_[2] = edamame_->GetPointLight();

    areaLights_[2] = vendingMac_->GetAreaLight();
    areaLights_[3] = wallManager_->GetAreaLight();
    areaLights_[4] = wallManager2_->GetAreaLight();
    areaLights_[5] = elevatorRoomManager_->GetAreaLight();

    Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
    Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
    Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
    Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeAreaLightCount_);
    Object3dCommon::GetInstance()->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
#pragma endregion

#ifdef USE_IMGUI
    if (ImGui::TreeNode("Light")) {
        ImGui::DragFloat3("Area0Position", &areaLights_[0].position.x, 0.1f);
        ImGui::DragFloat3("Area1Position", &areaLights_[1].position.x, 0.1f);
        ImGui::Checkbox("DirectionalShadow", &useDirectionalShadow_);
        ImGui::Checkbox("PointShadow", &usePointShadow_);
        ImGui::Checkbox("SpotShadow", &useSpotShadow_);
        ImGui::Checkbox("AreaShadow", &useAreaShadow_);
        pointLights_[0].shadowEnabled = usePointShadow_ ? 1 : 0;
        spotLights_[0].shadowEnabled = useSpotShadow_ ? 1 : 0;
        areaLights_[0].shadowEnabled = useAreaShadow_ ? 1 : 0;
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
    const bool shadowFlags[4] = { useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_ };
    for (int i = 0; i < 4; ++i) {
        if (!shadowFlags[i]) {
            continue;
        }
        object3dCommon->SetShadowMapEnabled(i == 0, i == 1, i == 2, i == 3);
        object3dCommon->BeginShadowMapPass();
        object3dCommon->DrawCommonShadow();
        DrawGameObject(true, false, false, true);
        object3dCommon->EndShadowMapPass();
    }
    object3dCommon->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
    //=======================shadowマップの終了↑=======================

    for (auto& portal : portalManager_->GetPortals()) {
        portal->BeginRender();
        auto* portalCamera = portal->GetCamera();
        SetCameraAndDraw(portalCamera, false, false, true);
        portal->TransitionToShaderResource();

    }

    Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
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