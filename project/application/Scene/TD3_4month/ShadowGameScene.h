#pragma once
#include "BaseScene.h"
#include <memory>
#include <imgui.h>
#include "SceneTransition/SceneTransition.h"

#include"GameObject/CameraController/CameraController.h"

#pragma region //GameObject
#include"GameObject/Player/Player.h"


#include"GameObject/Elevator/ElevatorRoomManager.h"
#include "GameObject/Elevator/Elevator.h"
#include "GameObject/Gentleman/Gentleman.h"
#pragma endregion
#include "GameObject/UI/DamageOverlay.h"


#include "GameObject/UI/UIManager.h" 
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"


#include "Light/CommonLight/DirectionalCommonLight.h" 
#include "Light/CommonLight/AreaCommonLight.h"
#include "Light/CommonLight/SpotCommonLight.h" 
#include "Light/CommonLight/PointCommonLight.h"
#include "Stages/StageManager.h"
#include"Audio.h"
#include <string>

class ShadowGameScene : public BaseScene {
private:


	std::unique_ptr<UIManager> uiManager_ = nullptr;

    // カメラの設定
	CameraController* cameraController_ = nullptr;
#pragma region // シーン遷移の設定
	// シーン遷移の設定
	std::unique_ptr<SceneTransition> transition_ = nullptr;
	// 遷移入り
	bool isTransitionIn_ = false;
	// 遷移抜け
	bool isTransitionOut_ = false;
	// 遷移完了後の次シーン名
	std::string nextSceneName_;
#pragma endregion

#pragma region//ゲームオブジェクトの設定
    //Player
    std::unique_ptr<Player> player_ = nullptr;
 
    //エレベータ部屋の管理
    std::unique_ptr<ElevatorRoomManager> elevatorRoomManager_ = nullptr;
    // エレベーター
    std::unique_ptr<Elevator> elevator_ = nullptr;
	// セーブポイント紳士
	std::unique_ptr<Gentleman> gentleman_ = nullptr;
	// 衝突管理
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
#pragma region// light
    //DirectionalLight
    DirectionalCommonLight directionalLight_{};

#pragma endregion
	bool useDirectionalShadow_ = true;

 	float playerHp_ = 3.0f;
	static constexpr float kPlayerMaxHp_ = 3.0f;
	float damageCooldownTimer_ = 0.0f;
	std::unique_ptr<DamageOverlay> damageOverlay_ = nullptr;
	// ステージ管理
	std::unique_ptr<StageManager> stageManager_ = nullptr;

public:
	// シーンのコンストラクタ
	ShadowGameScene();
	// デストラクタ
	~ShadowGameScene() override;
	// 初期化処理
	void Initialize() override;
	// 更新処理
	void Update() override;
	// 描画処理
	void Draw() override;
	// 終了処理
	void Finalize() override;
	// デバック
	void DebugImGui();
	// 衝突判定チェック
	void CheckCollision();

private:
	// =======================================
	// プライベート初期化
	// =======================================
	void InitializeLights();
	// =======================================
	// プライベート更新処理
	// =======================================
	// カメラの更新処理
	void UpdateCamera();
	// シーン遷移の更新処理
	void UpdateSceneTransition();
	// ポストエフェクトの更新処理
	void UpdatePostEffect();
	// ゲームオブジェクトの更新処理
	void UpdateGameObject();
	void UpdatePlayerDamage();
	void ApplyPlayerDamage(float damageAmount);
    //ポイントライトの更新処理
    void UpdateLight();
    // =======================================
    // プライベート描画処理
    // =======================================
    //シーン遷移の描画処理
    void DrawSceneTransition();
    //ゲームオブジェクトの描画処理
    void DrawModel();
    void DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle, bool drawPlayer);
    void SetSceneCameraForDraw(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    void SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle, bool drawPlayer);
};

