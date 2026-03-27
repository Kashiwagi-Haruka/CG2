#include "ShadowGameScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"
#include<numbers>
#include"RigidBody.h"
#include "WinApp.h"
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
    //デバックカメラ
    debugCamera_ = std::make_unique<DebugCamera>();
    //プレイヤーの生成
    player_ = std::make_unique<Player>();
    //プレイヤー視点のカメラ
    playerCamera_ = std::make_unique<PlayerCamera>();
    playerCamera_->SetPlayer(player_.get());
    //テスト地面
    testField_ = std::make_unique<TestField>();
    //ホワイトボード管理
    portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate);
    portalManager_->SetPlayerCamera(playerCamera_.get());

    //携帯打刻機
    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    //懐中電灯
    flashlight_ = std::make_unique<Flashlight>();
    flashlight_->SetPlayer(player_.get());
    // 鍵管理
    key_ = std::make_unique<Key>();
    // 枝豆管理
    edamame_ = std::make_unique<Edamame>();
    //ドア
    door_ = std::make_unique<Door>();
    //ロッカー
    lockerManager_ = std::make_unique<LockerManager>();
    //壁管理
    wallManager_ = std::make_unique<WallManager>();
    //壁管理
    wallManager2_ = std::make_unique<WallManager>();
    //自販機
    vendingMac_ = std::make_unique<VendingMac>();
    //椅子
    chairManager_ = std::make_unique<ChairManager>();
    //机
    deskManager_ = std::make_unique<DeskManager>();
    //打刻機
    timeCard_ = std::make_unique<TimeCard>();
    //タイムカードラック
    timeCardRack_ = std::make_unique<TimeCardRack>();
    //衝突管理
    collisionManager_ = std::make_unique<CollisionManager>();

    portalManager_->SetPlayerCamera(playerCamera_.get());
    //Playerの座標のポインタを入れる
    timeCardWatch_->SetPlayer(player_.get());
    //UIManager
    textUIManager_ = std::make_unique<TextUIManager>();

    key_->SetPlayerCamera(playerCamera_.get());
    edamame_->SetPlayerCamera(playerCamera_.get());
    chairManager_->SetPlayerCamera(playerCamera_.get());
    vendingMac_->SetPlayerCamera(playerCamera_.get());
    door_->SetPlayerCamera(playerCamera_.get());
    flashlight_->SetPlayerCamera(playerCamera_.get());
    lockerManager_->SetPlayerCamera(playerCamera_.get());
    deskManager_->SetPlayerCamera(playerCamera_.get());
}

ShadowGameScene::~ShadowGameScene()
{
    //BGMの管理
    BGMManager::UnLoad();
}

void ShadowGameScene::Initialize()
{

    //UIManager
    textUIManager_->Initialize();
    BGMManager::Initialize();
    isPause_ = false;
    noiseTimer_ = kNoiseTimer_;
    isNoise_ = false;

    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;

    //プレイヤーの初期化
    player_->Initialize();
    PlayerCommand::Initialize();
    playerCamera_->Initialize();

    //デバックカメラの設定
    debugCamera_->Initialize();
    debugCamera_->SetTranslation(playerCamera_->GetTransform().translate);

    InitializeLights();

    //テスト地面
    testField_->Initialize();
    //ホワイトボード管理
    portalManager_->Initialize();
    //携帯打刻機
    timeCardWatch_->Initialize();
    // 鍵
    key_->Initialize();
    // 枝豆
    edamame_->Initialize();
    //椅子
    chairManager_->Initialize();
    //壁
    wallManager_->Initialize();
    //壁
    wallManager2_->Initialize();
    //自販機
    vendingMac_->Initialize();
    //ドア
    door_->Initialize();
    //ロッカー
    lockerManager_->Initialize();
    //デスク管理
    deskManager_->Initialize();
    //打刻機
    timeCard_->Initialize();
    //タイムカードラック
    timeCardRack_->Initialize();

    //カーソルを画面中央に設定する
    auto* input = Input::GetInstance();
    input->SetIsCursorVisible(false);
    input->SetIsCursorStability(true);

    SetSceneCameraForDraw(playerCamera_->GetCamera());

}

void ShadowGameScene::Update()
{


    //カーソルを画面中央に設定する
    auto* input = Input::GetInstance();

    if (input->TriggerKey(DIK_TAB)) {
        //Tabキーでポーズ
        isPause_ = (isPause_) ? false : true;

        if (isPause_) {
            input->SetIsCursorVisible(true);
            input->SetIsCursorStability(false);
        } else {
            input->SetIsCursorVisible(false);
            input->SetIsCursorStability(true);
        }
    }



    //if (isPause_) {
    //    return;
    //}

    //シーン遷移の更新処理
    UpdateSceneTransition();
    //カメラの更新処理
    UpdateCamera();

    //ライトの更新処理
    UpdateLight();
    //ゲームオブジェクトの更新処理
    UpdateGameObject();

    //オブジェクトの当たり判定
    CheckCollision();
    //Text
    textUIManager_->Update();

}

void ShadowGameScene::Draw()
{
    //ゲームオブジェクトの描画処理
    DrawModel();

    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
    playerCamera_->DrawRaySprite();

    textUIManager_->Draw();

    //シーン遷移の描画処理
    DrawSceneTransition();
}

void ShadowGameScene::Finalize()
{
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
    //ホワイトボードとrayの当たり判定作成する
    portalManager_->CheckCollision();
    door_->CheckCollision();
    vendingMac_->CheckCollision();

    collisionManager_->ClearColliders();

    collisionManager_->AddCollider(player_.get());


    for (auto& portal : portalManager_->GetPortals()) {
        if (!portal->GetIsPlayerHit()) {
            //プレイヤーがヒットしてないときコライダーリストに追加する
            collisionManager_->AddCollider(portal.get());
        } else {
            break;
        }
    }

    for (auto& whiteBoard : portalManager_->GetWhiteBoards()) {
        collisionManager_->AddCollider(whiteBoard.get());
    }
    for (auto& wall : wallManager_->GetWalls()) {
        collisionManager_->AddCollider(wall.get());
    }

    for (auto& wall : wallManager2_->GetWalls()) {
        collisionManager_->AddCollider(wall.get());
    }

    collisionManager_->AddCollider(vendingMac_.get());
    collisionManager_->AddCollider(flashlight_.get());
    collisionManager_->AddCollider(testField_.get());
    for (auto& chair : chairManager_->GetChairs()) {
        collisionManager_->AddCollider(chair.get());
    }

    collisionManager_->AddCollider(door_->GetAutoLockSystem().get());

    if (!door_->GetIsOpen()) {
        collisionManager_->AddCollider(door_.get());
    }

    for (auto& locker : lockerManager_->GetLockers()) {
        collisionManager_->AddCollider(locker.get());
    }
    for (auto& desk : deskManager_->GetDesks()) {
        collisionManager_->AddCollider(desk.get());
    }
    collisionManager_->AddCollider(key_.get());

    collisionManager_->CheckAllCollisions();
}

void ShadowGameScene::InitializeLights()
{
    //フラッシュライト
    flashlight_->Initialize();

    spotLights_[1] = flashlight_->GetSpotLight();
    areaLights_[2] = vendingMac_->GetAreaLight();

    activePointLightCount_ = 2;
    pointLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLights_[0].position = { 7.0f, 0.0f, 0.0f };
    pointLights_[0].intensity = 1.0f;
    pointLights_[0].radius = 10.0f;
    pointLights_[0].decay = 1.0f;
    pointLights_[1].color = { 1.0f, 0.0f, 0.0f, 1.0f };
    pointLights_[1].position = { 5.0f, 5.0f, 5.0f };
    pointLights_[1].intensity = 1.0f;
    pointLights_[1].radius = 10.0f;
    pointLights_[1].decay = 1.0f;

    directionalLight_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLight_.direction = { 0.0f, 1.0f, 0.0f };
    directionalLight_.intensity = 0.1f;

    activeSpotLightCount_ = 2;
    spotLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    spotLights_[0].position = { 2.0f, 1.25f, 0.0f };
    spotLights_[0].direction = { -1.0f, -1.0f, 0.0f };
    spotLights_[0].intensity = 4.0f;
    spotLights_[0].distance = 7.0f;
    spotLights_[0].decay = 2.0f;
    spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

    activeAreaLightCount_ = 5;
    areaLights_[0].color = { 1.0f,1.0f, 1.0f, 1.0f };
    areaLights_[0].position = { 7.0f, 3.0f, 0.0f };
    areaLights_[0].normal = { 0.0f, 1.0f, 0.0f };
    areaLights_[0].intensity = 10.0f;
    areaLights_[0].width = 2.0f;
    areaLights_[0].height = 2.0f;
    areaLights_[0].radius = 5.0f;
    areaLights_[0].decay = 2.0f;

    areaLights_[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[1].position = { -7.0f, 3.0f, 0.0f };
    areaLights_[1].normal = { 0.0f, 1.0f, 0.0f };
    areaLights_[1].intensity = 10.0f;
    areaLights_[1].width = 2.0f;
    areaLights_[1].height = 2.0f;
    areaLights_[1].radius = 5.0f;
    areaLights_[1].decay = 2.0f;

}
#pragma region //private更新処理
void ShadowGameScene::UpdateCamera()
{
    if (useDebugCamera_) {
        debugCamera_->Update();
        playerCamera_->GetCamera()->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
    }
    Object3dCommon::GetInstance()->SetDefaultCamera(playerCamera_->GetCamera());
#ifdef USE_IMGUI
    if (ImGui::Begin("Camera")) {
        ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
        ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
        if (ImGui::TreeNode("Transform")) {

            if (!useDebugCamera_) {
                auto& playerCameraT = player_->GetTransform();
                ImGui::DragFloat3("Scale", &playerCameraT.scale.x, 0.01f);
                ImGui::DragFloat3("Rotate", &playerCameraT.rotate.x, 0.01f);
                ImGui::DragFloat3("Translate", &playerCameraT.translate.x, 0.01f);
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

#endif
}

void ShadowGameScene::UpdateSceneTransition()
{
    if (door_->GetOpenMassage()) {
        transition_->Initialize(false);
        isTransitionOut_ = true;
    }

    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Update();
        if (transition_->IsEnd() && isTransitionIn_) {
            isTransitionIn_ = false;
        }
        if (transition_->IsEnd() && isTransitionOut_) {
            //シーンの切り替え
            SceneManager::GetInstance()->ChangeScene("Result");
        }
    }
}

void ShadowGameScene::UpdateGameObject()
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

#pragma region//ゲームオブジェクト



    spotLights_[1] = flashlight_->GetSpotLight();
    areaLights_[2] = vendingMac_->GetAreaLight();
    areaLights_[3] = wallManager_->GetAreaLight();
    areaLights_[4] = wallManager2_->GetAreaLight();
    spotLights_[1] = flashlight_->GetSpotLight();
    areaLights_[2] = vendingMac_->GetAreaLight();

    if (!useDebugCamera_) {
        playerCamera_->Update();
    }

    portalManager_->WarpPlayer(player_.get());
    //プレイヤー
    player_->Update();

    //携帯打刻機
    timeCardWatch_->Update();
    //懐中電灯
    flashlight_->Update();
    //鍵
    key_->Update();
    //枝豆管理
    edamame_->Update();
    //椅子管理
    chairManager_->Update();
    //床
    testField_->Update();
    //壁管理
    wallManager_->Update();
    //壁管理
    wallManager2_->Update();
    //自販機
    vendingMac_->Update();
    //ドア
    door_->Update();
    //ロッカー
    lockerManager_->Update();
    //机
    deskManager_->Update();
    //ポータル管理
    portalManager_->Update();
    //打刻機
    timeCard_->SetTransform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f}, { 8.0f, 1.0f, -7.0f } });
    timeCard_->Update();
    //タイムカードラック
    timeCardRack_->SetTransform({ { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 7.75f,1.3f,-7.0f } });
    timeCardRack_->Update();



    if (*key_->GetKeyPtr()) {
        door_->SetIsGetKey(key_->GetKeyPtr());
    }

    for (auto& chair : chairManager_->GetChairs()) {
        if (chair->GetIsStand()) {
            Vector3 pos = chair->GetWorldPosition();
            pos.y += 1.0f;
            player_->SetTranslate(pos);
            chair->SetIsStand(false);
            PlayerCommand::SetIsStand(false);
            break;
        }
    }

    ParticleManager::GetInstance()->Update(playerCamera_->GetCamera());
#pragma endregion
}
void ShadowGameScene::UpdateLight() {
#pragma region // Lightを組み込む
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
    SetCameraAndDraw(playerCamera_->GetCamera(), true, true, false);
}
void ShadowGameScene::DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle, bool drawPlayer)
{

    // テスト地面
    testField_->Draw();
    //壁管理
    wallManager_->Draw();
    //壁管理
    wallManager2_->Draw();
    //自販機
    vendingMac_->Draw();
    //ドア
    door_->Draw();
    //ロッカー
    lockerManager_->Draw();
    //机
    deskManager_->Draw();
    //携帯打刻機の描画処理
    timeCardWatch_->Draw();
    //懐中電灯
    flashlight_->Draw();
    // 鍵の描画処理
    key_->Draw();
    // 枝豆の描画処理
    edamame_->Draw();
    //椅子の描画
    chairManager_->Draw();
    //打刻機
    timeCard_->Draw();
    //タイムカードラック
    timeCardRack_->Draw();

    if (!isShadow) {
        Object3dCommon::GetInstance()->DrawCommonSkinning();
    }

    if (drawPlayer) {
        // プレイヤーの描画処理
        player_->Draw();
    }

    //ポータル管理の描画
    portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}

void ShadowGameScene::SetSceneCameraForDraw(Camera* camera)
{
    player_->SetCamera(camera);
    testField_->SetCamera(camera);
    portalManager_->SetCamera(camera);
    timeCardWatch_->SetCamera(camera);
    flashlight_->SetCamera(camera);
    key_->SetCamera(camera);
    edamame_->SetCamera(camera);
    chairManager_->SetCamera(camera);
    wallManager_->SetCamera(camera);
    wallManager2_->SetCamera(camera);
    vendingMac_->SetCamera(camera);
    door_->SetCamera(camera);
    lockerManager_->SetCamera(camera);
    deskManager_->SetCamera(camera);
    timeCard_->SetCamera(camera);
    timeCardRack_->SetCamera(camera);
}
void ShadowGameScene::SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle, bool drawPlayer)
{
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);
    SetSceneCameraForDraw(camera);
    Object3dCommon::GetInstance()->DrawCommon();
    DrawGameObject(false, drawPortal, isDrawParticle, drawPlayer);
}
#pragma endregion