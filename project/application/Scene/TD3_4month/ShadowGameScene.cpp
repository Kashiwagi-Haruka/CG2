#define NOMINMAX
#define NOMINMAX
#include "ShadowGameScene.h"
#include"GameObject/GentleMan/GiantEnemyManager.h"

#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

#include"GameObject/KeyBindConfig.h"
#include "Particle/ParticleManager.h"
#include"GameObject/BGMManager/BGMManager.h"
#include"GameSave/GameSave.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include "GameObject/UI/Inventory/Inventory.h"
#include "Stages/StageNumber.h"
#include"GameBase.h"
#include"Color/Color.h"

#include"Model/ModelManager.h"
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

    auto saveData = gameSave.GetProgressSaveData();

    // カメラコントローラー
    cameraController_ = CameraController::GetInstance();
    cameraController_->SetPlayer(player_.get());
    //ライト管理
    lightManager_ = std::make_unique<Yoshida::LightManager>();
    //ステージ管理
    stageManager_ = std::make_unique<StageManager>(player_.get());
    stageManager_->CreateStage(saveData.currentStageName);
    // エレベーター
    elevator_ = std::make_unique<Elevator>();
    //紳士管理
    gentleManManager_ = std::make_unique<GentleManManager>();
    gentleManManager_->SetPlayer(player_.get());

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
    //ダメージ
    damageOverlay_ = std::make_unique<DamageOverlay>();
    //スカイボックス
    skyBox_ = std::make_unique<SkyBox>();

    //携帯打刻機　
    for (auto& timeCardWatch : timeCardWatches_) {
        timeCardWatch = std::make_unique<TimeCardWatch>();
    }
    //一つはプレイやー一つは紳士用
    timeCardWatches_[0]->SetParentMat(&player_->GetHandMatPtr());
    timeCardWatches_[1]->SetParentMat(&gentleManManager_->GetGentleman()->GetHandMat());

    //建物
    buildings_ = std::make_unique<BuildingClass>();
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

    // プレイヤーの初期化
    player_->Initialize();

    PlayerCommand::Initialize();
    //カメラコントローラー
    cameraController_->Initialize();
    cameraController_->GetInstance()->GetPlayerCamera()->SetParam(gameSave.GetCameraSaveData());
    //ライト管理
    lightManager_->Initialize();


    auto progress = gameSave.GetProgressSaveData();
    //紳士管理
    gentleManManager_->Initialize();
    gentleManManager_->GetGentleman()->SetGentleManTackScript(progress.currentStageName);
    // エレベータールーム
    elevatorRoomManager_->Initialize();
    // エレベーター
    elevator_->Initialize();
    elevator_->SetStageNumber(StageNumber::FromStageName(progress.currentStageName));

    for (auto& timeCardWatch : timeCardWatches_) {
        timeCardWatch->Initialize();
    }

    hierarchy->LoadObjectEditorsFromJsonIfExists("ShadowGameScene_objectEditors.json");
    hierarchy->EndRegisterFile();

    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/bill2", "bill2");

    std::vector<BuildingClass::Datas>datas(6);

    datas[0] = { .pos = {25.0f,0.0f,20.0f},.rotY = -1.572f };
    datas[1] = { .pos = {25.0f,0.0f,-20.0f},.rotY = -1.572f };
    datas[2] = { .pos = {25.0f,0.0f,0.0f},.rotY = -1.572f };

    datas[3] = { -datas[0].pos,-datas[0].rotY };
    datas[4] = { -datas[1].pos,-datas[1].rotY };
    datas[5] = { -datas[2].pos,-datas[2].rotY };

    buildings_->Initialize("bill2", datas);

    stageManager_->SetPlayerCamera(cameraController_->GetPlayerCamera());
    stageManager_->SetLightManager(lightManager_.get());
    stageManager_->InitializeStage();

    lightManager_->SetPointLight(cameraController_->GetPlayerCamera()->GetPointLight(), 0);
    lightManager_->SetPointLight(elevator_->GetPointLights().at(0), 1);
    lightManager_->SetPointLight(elevator_->GetPointLights().at(1), 2);
    //elevatorのエリアライトを設定する
    lightManager_->SetAreaLight(elevator_->GetAreaLight(), 0);

    //カーソルを画面中央に設定する
    uiManager_->CursorHideAndStop();
    uiManager_->ShowKeyLostAtStageStartMessage();

    //カメラをセットする
    SetSceneCameraForDraw(cameraController_->GetPlayerCamera()->GetCamera());

    //最初のイベントをセットする
    currentEvent_ = firstEvent_.get();
    currentEvent_->StartEvent();

    Update();


}

void ShadowGameScene::Update() {
#ifdef USE_IMGUI
    DebugImGui();
#endif
    // BGMの更新処理
    BGMManager::Update();

    UpdateStagetransition();
    currentEvent_->Update();

    // ポストエフェクトの更新処理
    UpdatePostEffect();

    if (currentEvent_->IsRunning()) {
        //イベント中は一旦プレイヤー移動をロックする
        PlayerCommand::SetIsMoveLocked(true);

    } else {
        // UI管理
        uiManager_->Update();
        if (Inventory::GetInstance()->ConsumeItemUseEvent()) {
            player_->SetHP(player_->GetMaxHP());
        }
        PlayerCommand::SetIsMoveLocked(UIManager::IsUiOperationBlocked());
    }


    // ゲームオブジェクトの更新処理
    UpdateGameObject();
    // カメラの更新処理
    UpdateCamera();
    // ポータル管理 カメラの更新後に行う
    stageManager_->UpdatePortal();
    //プレイヤーのダメージ
    UpdatePlayerDamage();

    // ライトの更新処理
    UpdateLight();

    // オブジェクトの当たり判定
    CheckCollision();

    // シーン遷移の更新処理
    UpdateSceneTransition();
}

void ShadowGameScene::Draw()
{
    //ゲームオブジェクトの描画処理
    DrawModel();

    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
    //ステージマネージャーでスプライトを描画
    stageManager_->DrawSprite();

    auto progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    elevator_->DrawSprite();

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
    static constexpr const char* kStageNames[] = { "MirrorStage", "LightStage", "TutorialStage", "RadiconStage","GentleManStage","RestroomStage","ElevatorFallStage" ,"LoopStage" };
    int stageIndex = 0;
    auto& progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    if (progressSaveData.currentStageName == "LightStage") {
        stageIndex = 1;
    } else if (progressSaveData.currentStageName == "TutorialStage") {
        stageIndex = 2;
    } else if (progressSaveData.currentStageName == "RadiconStage") {
        stageIndex = 3;
    } else if (progressSaveData.currentStageName == "GentleManStage") {
        stageIndex = 4;
    } else if (progressSaveData.currentStageName == "RestroomStage") {
        stageIndex = 5;
    } else if (progressSaveData.currentStageName == "ElevatorFallStage") {
        stageIndex = 6;
    } else if (progressSaveData.currentStageName == "LoopStage") {
        stageIndex = 7;
    }

    if (ImGui::Combo("Stage", &stageIndex, kStageNames, IM_ARRAYSIZE(kStageNames))) {
        ChangeStage(kStageNames[stageIndex]);
    }

    ImGui::Text("isGameClear : %d", progressSaveData.isGameClear);
    ImGui::Text("isKeyHave : %d", progressSaveData.isKeyHave);
    ImGui::Text("isLightHave : %d", progressSaveData.isLightHave);


    ImGui::End();
#endif // USE_IMGUI
}

void ShadowGameScene::ChangeStage(const std::string& stageName) {

    auto& gameSave = GameSave::GetInstance();

    if (!gameSave.SetCurrentStage(stageName)) {
        return;
    }

    auto& progressSaveData = gameSave.GetProgressSaveData();

    //掴んでいたものをはなすなど
    ChairMenu::SetIsShowMenu(false);
    ChairManager::Reset();
    PlayerCommand::SetIsGrab(false);

    if (progressSaveData.currentStageName != "GentleManStage" && progressSaveData.currentStageName != "LoopStage") {

        //最終ステージ付近ではないとき鍵をなくす

        //キーをなくす
        gameSave.SetIsKeyHave(false);
        uiManager_->ShowKeyLostAtStageStartMessage();
    }


    stageManager_->CreateStage(progressSaveData.currentStageName);
    stageManager_->SetPlayerCamera(cameraController_->GetPlayerCamera());
    stageManager_->SetLightManager(lightManager_.get());
    stageManager_->SetPlayer(player_.get());
    stageManager_->SetCollisionManager(collisionManager_.get());
    stageManager_->InitializeStage();
    //elevatorの番号を変更する
    elevator_->SetStageNumber(StageNumber::FromStageName(progressSaveData.currentStageName));
    //紳士のトークを変更する
    gentleManManager_->GetGentleman()->SetGentleManTackScript(progressSaveData.currentStageName);

    SetSceneCameraForDraw(cameraController_->GetPlayerCamera()->GetCamera());
    
   
    //ゲームクリアを初期化する
    gameSave.SetIsGameClear(false);
}

void ShadowGameScene::CheckCollision() {

    if (!collisionManager_) {
        return;
    }

    collisionManager_->ClearColliders();

    collisionManager_->AddCollider(player_.get());


    if (!isEndStage()) {

        for (auto& [name, wall] : elevatorRoomManager_->GetColliders()) {
            collisionManager_->AddCollider(wall.get());
        }

        //最終ステージの時はセットしない
        for (auto& [name, system] : elevator_->GetAutoLockSys()) {
            collisionManager_->AddCollider(system.get());
        }
        for (auto& [name, collider] : elevator_->GetColliders()) {
            if (name == "ElevatorFloor" && elevator_->IsFall()) {
                continue;
            }
            collisionManager_->AddCollider(collider.get());
        }

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

    auto& progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    if (GiantEnemyManager::GetIsAllPortal()) {
        //巨人が前夫ポータルになったら
        if (isEndStage() && !isTransitionOut_) {
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

    if (isPlayerWarp()) {
        if (!isNoise_) {
            isNoise_ = true;
        }
    };


    if (isNoise_) {
        float randomNoiseScale = 1.0f;
        const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
        noiseTimer_ -= deltaTime;
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

    timeCardWatches_[0]->Update();

    auto& progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    if (progressSaveData.currentStageName != "GentleManStage") {
        timeCardWatches_[1]->Update();
    }

    stageManager_->SetPlayer(player_.get());
    stageManager_->UpdateGameObject(cameraController_->GetPlayerCamera()->GetCamera(), lightManager_->GetDirectionalLight().direction);

    UpdateElevator();


    //紳士管理
    gentleManManager_->Update();
    //建物 回数によって位置を変更する
    buildings_->Update(StageNumber::FromStageName(progressSaveData.currentStageName));
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
    //プレイヤー
    lightManager_->SetPointLight(cameraController_->GetPlayerCamera()->GetPointLight(), 0);
    //エレベータ
    lightManager_->SetPointLight(elevator_->GetPointLights().at(0), 1);
    lightManager_->SetPointLight(elevator_->GetPointLights().at(1), 2);
    lightManager_->SetAreaLight(elevator_->GetAreaLight(), 0);


}
void ShadowGameScene::UpdateElevator()
{


    if (isEndStage()) {
        return;
        //最終ステージだったらカエル
    }
    auto& progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    if (progressSaveData.currentStageName == "ElevatorFallStage" && !Key::IsGetKey()) {
        //落下アニメーションを設定する
        elevator_->SetFallAnimation();
    }

    // エレベーター
    elevator_->Update();

    // エレベータールーム管理
    elevatorRoomManager_->Update();
}
void ShadowGameScene::UpdateStagetransition()
{

    auto& gameSave = GameSave::GetInstance();
    auto& saveData = gameSave.GetProgressSaveData();
    //ゲームクリアを初期化する
    gameSave.SetIsKeyHave(Key::IsGetKey());
    gameSave.SetIsLightHave(Flashlight::IsGetLight());

    if (!saveData.isGameClear) {
        //未クリア時
        if (saveData.currentStageName == "GentleManStage") {
            //巨人ステージの時
            if (isPlayerWarp()) {
                gameSave.SetIsGameClear(true);
            }

        } else if (saveData.currentStageName == "LoopStage") {
            //LoopStage
            if (GiantEnemyManager::GetIsAllPortal()) {
                gameSave.SetIsGameClear(true);
            }

        } else {
            //その他ステージの時
            if (saveData.isKeyHave) {
                //仮に鍵を取得したときをゲームクリアとする
                gameSave.SetIsGameClear(true);
            }
        }
    }


    //最後のステージじゃないときステージの変更をする
    if (saveData.isGameClear && saveData.currentStageName != "LoopStage") {
        //巨人ステージの場合　elevatorを仲介しない
        if (saveData.currentStageName == "GentleManStage") {
            //ループステージ
            ChangeStage("LoopStage");
        } else {
            //ステージの切り替え
            if (elevator_->IsSceneTransitionStart()) {

                if (saveData.currentStageName == "MirrorStage") {
                    ChangeStage("TutorialStage");
                } else if (saveData.currentStageName == "TutorialStage") {
                    ChangeStage("RestroomStage");
                } else if (saveData.currentStageName == "RestroomStage") {
                    ChangeStage("ElevatorFallStage");
                } else if (saveData.currentStageName == "ElevatorFallStage") {
                    ChangeStage("GentleManStage");
                }

            }
        }
    }


}

#pragma endregion
#pragma region 
bool ShadowGameScene::isPlayerWarp()
{

    if (auto* portalManager = stageManager_->GetPortalManager()) {
        for (auto& portal : portalManager->GetPortals()) {
            if (portal->GetIsPlayerCanWarp()) {
                return true;
            }
        }
    }

    return false;
}
bool ShadowGameScene::isEndStage()
{
    auto progressSaveData = GameSave::GetInstance().GetProgressSaveData();


    // エレベーター
    if (progressSaveData.currentStageName == "LoopStage") {
        return true;
    }

    return false;

}
// private描画処理
void ShadowGameScene::DrawSceneTransition() {
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Draw();
    }
}

void ShadowGameScene::DrawModel() {
    //=======================shadowマップの開始↓=======================
    auto* object3dCommon = Object3dCommon::GetInstance();
    object3dCommon->SetShadowMapEnabled(true, false, false, false);
    object3dCommon->BeginShadowMapPass();
    object3dCommon->DrawCommonShadow();
    Object3dCommon::GetInstance()->SetDefaultCamera(cameraController_->GetPlayerCamera()->GetCamera());
    SetSceneCameraForDraw(cameraController_->GetPlayerCamera()->GetCamera());
    DrawGameObject(true, false, false, false, false);
    object3dCommon->EndShadowMapPass();

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

    auto progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    // エレベーター
    if (!isEndStage()) {
        elevator_->Draw();
    }

    gentleManManager_->Draw();

    if (progressSaveData.currentStageName != "GentleManStage") {
        timeCardWatches_[1]->Draw();
    }

    Object3dCommon::GetInstance()->DrawCommon();
    stageManager_->DrawModel(isShadow, drawPortal, isDrawParticle);

    if (drawPlayer) {

        timeCardWatches_[0]->Draw();
        // プレイヤーの描画処理
        player_->Draw();

    }

    Object3dCommon::GetInstance()->DrawCommon();

    if (!isEndStage()) {
        // エレベータルーム
        elevatorRoomManager_->Draw();
    }

    //建物
    buildings_->Draw();
}

void ShadowGameScene::SetSceneCameraForDraw(Camera* camera) {
    skyBox_->SetCamera(camera);
    player_->SetCamera(camera);

    auto progressSaveData = GameSave::GetInstance().GetProgressSaveData();

    if (!isEndStage()) {
        //最終ステージの時はセットしない
        elevatorRoomManager_->SetCamera(camera);
        elevator_->SetCamera(camera);
    }

    gentleManManager_->SetCamera(camera);
    buildings_->SetCamera(camera);

    for (auto& timeCardWatch : timeCardWatches_) {
        timeCardWatch->SetCamera(camera);
    }

    stageManager_->SetSceneCameraForDraw(camera);

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
