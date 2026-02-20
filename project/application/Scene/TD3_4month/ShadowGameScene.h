#pragma once
#include "BaseScene.h"
#include <memory>
#include <imgui.h>
#include "SceneTransition/SceneTransition.h"
#include "Camera.h"
#include "DebugCamera.h"
#include"GameObject/Player/Player.h"
#include"GameObject/TestField/TestField.h"

#include "Light/DirectionalLight.h" 
#include "Light/PointLight.h"
#include "Light/SpotLight.h" 
#include "Light/AreaLight.h"
class ShadowGameScene : public BaseScene
{
private:
    Transform cameraTransform_ = {};
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
    bool useDebugCamera_ = false;

    //シーン遷移の設定
    std::unique_ptr<SceneTransition> transition_ = nullptr;
    bool isTransitionIn_ = false;
    bool isTransitionOut_ = false;

    //Player
    std::unique_ptr<Player> player_ = nullptr;
    //TestField
    std::unique_ptr<TestField> testField_ = nullptr;
#pragma region// light
    //DirectionalLight
    DirectionalLight directionalLight_{}; 
    //PointLight
    std::array<PointLight, kMaxPointLights> pointLights_{};
    uint32_t activePointLightCount_ = 0;
    //SpotLight
    std::array<SpotLight, kMaxSpotLights> spotLights_{};
    uint32_t activeSpotLightCount_ = 0;
    //AreaLight
    std::array<AreaLight, kMaxAreaLights> areaLights_{};
    uint32_t activeAreaLightCount_ = 0;
#pragma endregion
public:
    ShadowGameScene();
    ~ShadowGameScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    void DebugImGui();
private:
    // =======================================
    // プライベート初期化
    // =======================================
    void InitializeLights();
    // =======================================
    // プライベート更新処理
    // =======================================
    //カメラの更新処理
    void UpdateCamera();
    //シーン遷移の更新処理
    void UpdateSceneTransition();
    //ゲームオブジェクトの更新処理
    void UpdateGameObject();
    //ポイントライトの更新処理
    void UpdatePointLight();
    // =======================================
    // プライベート描画処理
    // =======================================
    //シーン遷移の描画処理
    void DrawSceneTransition();
    //ゲームオブジェクトの描画処理
    void DrawGameObject();

};

