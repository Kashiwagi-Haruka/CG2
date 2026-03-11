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

ShadowGameScene::ShadowGameScene()
{
    //シーン遷移の設定
    transition_ = std::make_unique<SceneTransition>();
    //デバックカメラ
    debugCamera_ = std::make_unique<DebugCamera>();
    //プレイヤーの生成
    player_ = std::make_unique<Player>();
    //プレイヤー視点のカメラ
    playerCamera_ = std::make_unique<PlayerCamera>();
    playerCamera_->SetPlayerTransformPtr(&player_->GetTransform());
    //テスト地面
    testField_ = std::make_unique<TestField>();
    //SEを読み込む
    Portal::LoadSE();
    //ホワイトボード管理
    portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate);
    portalManager_->SetPlayerCamera(playerCamera_.get());

    //携帯打刻機
    timeCardWatch_ = std::make_unique<TimeCardWatch>();
    //懐中電灯
    flashlight_ = std::make_unique<Flashlight>();
    // 鍵管理
    key_ = std::make_unique<Key>();
    // 枝豆管理
    edamame_ = std::make_unique<Edamame>();
    //椅子
    chair_ = std::make_unique<Chair>();
    //壁管理
    wallManager_ = std::make_unique<WallManager>();
    //衝突管理
    collisionManager_ = std::make_unique<CollisionManager>();
}

ShadowGameScene::~ShadowGameScene()
{
    Portal::UnLoadSE();
}

void ShadowGameScene::Initialize()
{
    isPause_ = false;
    noiseTimer_ = kNoiseTimer_;
    isNoise_ = false;

    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;

    playerCamera_->Initialize();

    //デバックカメラの設定
    debugCamera_->Initialize();
    debugCamera_->SetTranslation(playerCamera_->GetTransform().translate);
    //プレイヤーの初期化
    player_->Initialize();
    //テスト地面
    testField_->Initialize();

    InitializeLights();

    //ホワイトボード管理
    portalManager_->Initialize();
    portalManager_->SetPlayerCamera(playerCamera_.get());


    //携帯打刻機
    timeCardWatch_->Initialize();
    //Playerの座標のポインタを入れる
    timeCardWatch_->SetTransformPtr(&player_->GetTransform());

    // 鍵
    key_->Initialize();
    key_->SetPlayerCamera(playerCamera_.get());

    // 枝豆
    edamame_->Initialize();
    edamame_->SetPlayerCamera(playerCamera_.get());

    //椅子
    chair_->Initialize();
    chair_->SetPlayerCamera(playerCamera_.get());
    //壁
    wallManager_->Initialize();

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

    if (isPause_) {
        return;
    }

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
}

void ShadowGameScene::Draw()
{
    //ゲームオブジェクトの描画処理
    DrawModel();

    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
    playerCamera_->DrawRaySprite();
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
    portalManager_->CheckCollision(timeCardWatch_.get());
    key_->CheckCollision();

    edamame_->CheckCollision();
    chair_->CheckCollision();

    collisionManager_->ClearColliders();

    collisionManager_->AddCollider(player_.get());
    collisionManager_->AddCollider(chair_.get());


    for (auto& portal : portalManager_->GetPortals()) {
        collisionManager_->AddCollider(portal.get());
    }

    for (auto& whiteBoard : portalManager_->GetWhiteBoards()) {
        collisionManager_->AddCollider(whiteBoard.get());
    }
    for (auto& wall : wallManager_->GetWalls()) {
        collisionManager_->AddCollider(wall.get());
    }
    collisionManager_->AddCollider(flashlight_.get());
    collisionManager_->AddCollider(testField_.get());

    collisionManager_->SetCamera(playerCamera_->GetCamera());

    collisionManager_->CheckAllCollisions();
}

void ShadowGameScene::InitializeLights() {
	// 懐中電灯
	flashlight_->Initialize();
	flashlight_->SetCamera(playerCamera_->GetCamera());

	pointLights_.ClearLights();
	pointLights_.AddPointLight("Point0");
	pointLights_.SetLightProperties("Point0", {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 5.0f, 0.0f}, 1.0f, 10.0f, 1.0f);
	pointLights_.AddPointLight("Point1");
	pointLights_.SetLightProperties("Point1", {1.0f, 0.0f, 0.0f, 1.0f}, {5.0f, 5.0f, 5.0f}, 1.0f, 10.0f, 1.0f);

	directionalLight_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	directionalLight_.SetDirection({0.0f, 1.0f, 0.0f});
	directionalLight_.SetIntensity(0.1f);

	spotLights_.ClearSpotLights();
	spotLights_.AddSpotLight("Spot0");
	spotLights_.SetSpotLightProperties(
	    "Spot0", {1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 1.25f, 0.0f}, 4.0f, {-1.0f, -1.0f, 0.0f}, 7.0f, 2.0f, std::numbers::pi_v<float> / 3.0f, std::numbers::pi_v<float> / 4.0f);
	spotLights_.AddSpotLight("Spot1");

	areaLights_.ClearAreaLights();
	areaLights_.AddAreaLight("Area0");
	areaLights_.SetAreaLightColor("Area0", {1.0f, 1.0f, 1.0f, 1.0f});
	areaLights_.SetAreaLightPosition("Area0", {0.0f, 3.0f, 0.0f});
	areaLights_.SetAreaLightNormal("Area0", {1.0f, -1.0f, 0.0f});
	areaLights_.SetAreaLightIntensity("Area0", 4.0f);
	areaLights_.SetAreaLightSize("Area0", 2.0f, 2.0f);
	areaLights_.SetAreaLightDistance("Area0", 0.1f);
	areaLights_.SetAreaLightDecay("Area0", 2.0f);

	areaLights_.AddAreaLight("Area1");
	areaLights_.SetAreaLightColor("Area1", {1.0f, 1.0f, 1.0f, 1.0f});
	areaLights_.SetAreaLightPosition("Area1", {-5.0f, 3.0f, 0.0f});
	areaLights_.SetAreaLightNormal("Area1", {1.0f, -1.0f, 0.0f});
	areaLights_.SetAreaLightIntensity("Area1", 4.0f);
	areaLights_.SetAreaLightSize("Area1", 2.0f, 2.0f);
	areaLights_.SetAreaLightDistance("Area1", 0.1f);
	areaLights_.SetAreaLightDecay("Area1", 2.0f);
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
    if (Input::GetInstance()->TriggerKey(DIK_SPACE) && !isTransitionOut_) {
        transition_->Initialize(true);
        isTransitionOut_ = true;
    }
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Update();
        if (transition_->IsEnd() && isTransitionIn_) {
            isTransitionIn_ = false;
        }
        if (transition_->IsEnd() && isTransitionOut_) {
            //シーンの切り替え
     /*       SceneManager::GetInstance()->ChangeScene("Title");*/
        }
    }
}

void ShadowGameScene::UpdateGameObject()
{
#pragma region//Lightを組み込む
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_);
	Object3dCommon::GetInstance()->SetAreaLights(areaLights_);
#pragma endregion

    bool vignetteStrength = true;

    Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(false);
    Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(false);
    Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength);
    Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength);

    if (PlayerCommand::GetInstance()->Shot()) {
        if (!isNoise_) {
            isNoise_ = true;
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

    portalManager_->WarpPlayer(player_.get());

    if (!useDebugCamera_) {
        playerCamera_->Update();
    }
    //プレイヤー
    player_->Update();
    //携帯打刻機
    timeCardWatch_->Update();
    //鍵
    key_->Update();
    //枝豆管理
    edamame_->Update();
    //椅子管理
    chair_->Update();
    //床
    testField_->Update();
    //壁管理
    wallManager_->Update();
    //ポータル管理
    portalManager_->Update();
    ParticleManager::GetInstance()->Update(playerCamera_->GetCamera());
#pragma endregion
}
void ShadowGameScene::UpdateLight() {

	// 懐中電灯
	flashlight_->Update();
	flashlight_->ApplyToSpotLight(spotLights_, "Spot1");

#ifdef USE_IMGUI
	if (ImGui::TreeNode("PointLight")) {
		Vector4 point0Color = pointLights_.GetLightColor("Point0");
		if (ImGui::ColorEdit4("PointLightColor", &point0Color.x)) {
			pointLights_.SetLightColor("Point0", point0Color);
		}
		float point0Intensity = pointLights_.GetLightIntensity("Point0");
		if (ImGui::DragFloat("PointLightIntensity", &point0Intensity, 0.1f)) {
			pointLights_.SetLightIntensity("Point0", point0Intensity);
		}
		Vector3 point0Position = pointLights_.GetLightPosition("Point0");
		if (ImGui::DragFloat3("PointLightPosition", &point0Position.x, 0.1f)) {
			pointLights_.SetLightPosition("Point0", point0Position);
		}
		float point0Radius = pointLights_.GetLightRadius("Point0");
		if (ImGui::DragFloat("PointLightRadius", &point0Radius, 0.1f)) {
			pointLights_.SetLightRadius("Point0", point0Radius);
		}
		float point0Decay = pointLights_.GetLightDecay("Point0");
		if (ImGui::DragFloat("PointLightDecay", &point0Decay, 0.1f)) {
			pointLights_.SetLightDecay("Point0", point0Decay);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("PointLight1")) {
		Vector4 point1Color = pointLights_.GetLightColor("Point1");
		if (ImGui::ColorEdit4("PointLightColor1", &point1Color.x)) {
			pointLights_.SetLightColor("Point1", point1Color);
		}
		float point1Intensity = pointLights_.GetLightIntensity("Point1");
		if (ImGui::DragFloat("PointLightIntensity1", &point1Intensity, 0.1f)) {
			pointLights_.SetLightIntensity("Point1", point1Intensity);
		}
		Vector3 point1Position = pointLights_.GetLightPosition("Point1");
		if (ImGui::DragFloat3("PointLightPosition1", &point1Position.x, 0.1f)) {
			pointLights_.SetLightPosition("Point1", point1Position);
		}
		float point1Radius = pointLights_.GetLightRadius("Point1");
		if (ImGui::DragFloat("PointLightRadius1", &point1Radius, 0.1f)) {
			pointLights_.SetLightRadius("Point1", point1Radius);
		}
		float point1Decay = pointLights_.GetLightDecay("Point1");
		if (ImGui::DragFloat("PointLightDecay1", &point1Decay, 0.1f)) {
			pointLights_.SetLightDecay("Point1", point1Decay);
		}
		ImGui::TreePop();
	}
#endif
}
#pragma endregion

#pragma region //private描画処理
void ShadowGameScene::DrawSceneTransition()
{
    if (isTransitionIn_ || isTransitionOut_) {
        transition_->Draw();
    }
}

void ShadowGameScene::DrawModel()
{
    //=======================shadowマップの開始↓=======================
    Object3dCommon::GetInstance()->BeginShadowMapPass();
    Object3dCommon::GetInstance()->DrawCommonShadow();
    DrawGameObject(true, false, false);
    Object3dCommon::GetInstance()->EndShadowMapPass();
    //=======================shadowマップの終了↑=======================

    for (auto& portal : portalManager_->GetPortals()) {
        portal->BeginRender();
        auto* portalCamera = portal->GetCamera();
        SetCameraAndDraw(portalCamera, false, false);
        portal->TransitionToShaderResource();
    }

    Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
    SetCameraAndDraw(playerCamera_->GetCamera(), true, true);
}
void ShadowGameScene::DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle)
{

    // テスト地面
    testField_->Draw();
    //壁管理
    wallManager_->Draw();
    //携帯打刻機の描画処理
    timeCardWatch_->Draw();
    //懐中電灯
    flashlight_->Draw();
    // 鍵の描画処理
    key_->Draw();
    // 枝豆の描画処理
    edamame_->Draw();
    //椅子の描画
    chair_->Draw();

    if (!isShadow) {
        Object3dCommon::GetInstance()->DrawCommonSkinning();
    }

    // プレイヤーの描画処理
    player_->Draw();
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
    chair_->SetCamera(camera);
    wallManager_->SetCamera(camera);
}
void ShadowGameScene::SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle)
{
    Object3dCommon::GetInstance()->SetDefaultCamera(camera);
    SetSceneCameraForDraw(camera);
    Object3dCommon::GetInstance()->DrawCommon();
    DrawGameObject(false, drawPortal, isDrawParticle);
}
#pragma endregion