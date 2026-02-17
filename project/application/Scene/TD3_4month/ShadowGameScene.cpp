#include "ShadowGameScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include"Object3d/Object3dCommon.h"
#include"DirectXCommon.h"

ShadowGameScene::ShadowGameScene()
{
    //シーン遷移の設定
    transition_ = std::make_unique<SceneTransition>();
    //カメラの設定
    cameraTransform_ = {
    .scale{1.0f, 1.0f, 1.0f  },
    .rotate{0.0f, 0.0f, 0.0f  },
    .translate{0.0f, 5.0f, -10.0f}
    };
    camera_ = std::make_unique<Camera>();
    camera_->SetTransform(cameraTransform_);
    //デバックカメラ
    debugCamera_ = std::make_unique<DebugCamera>();
    //プレイヤーの生成
    player_ = std::make_unique<Player>();
}

ShadowGameScene::~ShadowGameScene()
{

}

void ShadowGameScene::Initialize()
{
    //シーン遷移の設定
    transition_->Initialize(false);
    isTransitionIn_ = true;
    isTransitionOut_ = false;
    //デバックカメラの設定
    debugCamera_->Initialize();
    debugCamera_->SetTranslation(cameraTransform_.translate);
    //プレイヤーの初期化
    player_->Initialize();
    player_->SetCamera(camera_.get());

}

void ShadowGameScene::Update()
{
    //シーン遷移の更新処理
    UpdateSceneTransition();
    //カメラの更新処理
    UpdateCamera();
    //ゲームオブジェクトの更新処理
    UpdateGameObject();
}

void ShadowGameScene::Draw()
{
    //スプライト共通
    SpriteCommon::GetInstance()->DrawCommon();
    //シーン遷移の描画処理
    DrawSceneTransition();
    //ゲームオブジェクトの描画処理
    DrawGameObject();
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
void ShadowGameScene::InitializeLights()
{
    activePointLightCount_ = 2;
    pointLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLights_[0].position = { 0.0f, 5.0f, 0.0f };
    pointLights_[0].intensity = 1.0f;
    pointLights_[0].radius = 10.0f;
    pointLights_[0].decay = 1.0f;
    pointLights_[1].color = { 1.0f, 0.0f, 0.0f, 1.0f };
    pointLights_[1].position = { 5.0f, 5.0f, 5.0f };
    pointLights_[1].intensity = 1.0f;
    pointLights_[1].radius = 10.0f;
    pointLights_[1].decay = 1.0f;

    directionalLight_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLight_.direction = { 0.0f, -1.0f, 0.0f };
    directionalLight_.intensity = 1.0f;

    activeSpotLightCount_ = 2;
    spotLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    spotLights_[0].position = { 2.0f, 1.25f, 0.0f };
    spotLights_[0].direction = { -1.0f, -1.0f, 0.0f };
    spotLights_[0].intensity = 4.0f;
    spotLights_[0].distance = 7.0f;
    spotLights_[0].decay = 2.0f;
    spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

    spotLights_[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    spotLights_[1].position = { 2.0f, 1.25f, 0.0f };
    spotLights_[1].direction = { -1.0f, -1.0f, 0.0f };
    spotLights_[1].intensity = 4.0f;
    spotLights_[1].distance = 7.0f;
    spotLights_[1].decay = 2.0f;
    spotLights_[1].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLights_[1].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

    activeAreaLightCount_ = 2;
    areaLights_[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[0].position = { 0.0f, 3.0f, 0.0f };
    areaLights_[0].normal = { 1.0f, -1.0f, 0.0f };
    areaLights_[0].intensity = 4.0f;
    areaLights_[0].width = 2.0f;
    areaLights_[0].height = 2.0f;
    areaLights_[0].radius = 0.1f;
    areaLights_[0].decay = 2.0f;

    areaLights_[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLights_[1].position = { -5.0f, 3.0f, 0.0f };
    areaLights_[1].normal = { 1.0f, -1.0f, 0.0f };
    areaLights_[1].intensity = 4.0f;
    areaLights_[1].width = 2.0f;
    areaLights_[1].height = 2.0f;
    areaLights_[1].radius = 0.1f;
    areaLights_[1].decay = 2.0f;
}
#pragma region //private更新処理
void ShadowGameScene::UpdateCamera()
{
    if (useDebugCamera_) {
        debugCamera_->Update();
        camera_->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
    } else {
        //Playerからの視点
        camera_->SetViewProjectionMatrix(player_->GetWorldMatrix(),camera_->GetProjectionMatrix());
    }
#ifdef USE_IMGUI
    if (ImGui::Begin("Camera")) {
        ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
        ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
        if (ImGui::TreeNode("Transform")) {

            if (!useDebugCamera_) {
                ImGui::DragFloat3("Scale", &cameraTransform_.scale.x, 0.01f);
                ImGui::DragFloat3("Rotate", &cameraTransform_.rotate.x, 0.01f);
                ImGui::DragFloat3("Translate", &cameraTransform_.translate.x, 0.01f);
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
    Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
    Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
    Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
    Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeAreaLightCount_);
   
    player_->Update();
}
void ShadowGameScene::UpdatePointLight()
{
#ifdef USE_IMGUI
    if (ImGui::TreeNode("PointLight")) {
        ImGui::ColorEdit4("PointLightColor", &pointLights_[0].color.x);
        ImGui::DragFloat("PointLightIntensity", &pointLights_[0].intensity, 0.1f);
        ImGui::DragFloat3("PointLightPosition", &pointLights_[0].position.x, 0.1f);
        ImGui::DragFloat("PointLightRadius", &pointLights_[0].radius, 0.1f);
        ImGui::DragFloat("PointLightDecay", &pointLights_[0].decay, 0.1f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("PointLight1")) {
        ImGui::ColorEdit4("PointLightColor1", &pointLights_[1].color.x);
        ImGui::DragFloat("PointLightIntensity1", &pointLights_[1].intensity, 0.1f);
        ImGui::DragFloat3("PointLightPosition1", &pointLights_[1].position.x, 0.1f);
        ImGui::DragFloat("PointLightRadius1", &pointLights_[1].radius, 0.1f);
        ImGui::DragFloat("PointLightDecay1", &pointLights_[1].decay, 0.1f);
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

void ShadowGameScene::DrawGameObject()
{
    Object3dCommon::GetInstance()->BeginShadowMapPass();
    Object3dCommon::GetInstance()->DrawCommonShadow();

    //プレイヤーの描画処理
    player_->Draw();

    Object3dCommon::GetInstance()->EndShadowMapPass();
    Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
    Object3dCommon::GetInstance()->DrawCommon();

    //Object3dCommon::GetInstance()->DrawCommonSkinningToon();
    //プレイヤーの描画処理
    player_->Draw();

}
#pragma endregion