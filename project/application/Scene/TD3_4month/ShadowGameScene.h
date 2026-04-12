#pragma once
#include "BaseScene.h"
#include <memory>
#include <imgui.h>
#include "SceneTransition/SceneTransition.h"

#include"GameObject/CameraController/CameraController.h"
#pragma region //Save
#include"GameSave/GameSave.h"
#pragma endregion
#pragma region //GameObject
#include"GameObject/Player/Player.h"

#include"GameObject/TestField/TestField.h"
#include"GameObject/Portal/PortalManager.h"
#include"GameObject/WhiteBoard/WhiteBoardManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include"GameObject/TimeCard/TimeCard.h"
#include"GameObject/TimeCard/TimeCardRack.h"
#include"GameObject/Flashlight/Flashlight.h"
#include"GameObject/Key/Key.h"
#include <GameObject/Chair/ChairManager.h>
#include"GameObject/Wall/WallManager.h"
#include"GameObject/Wall/WallManager2.h"
#include"GameObject/VendingMac/VendingMac.h"
#include"GameObject/Door/Door.h"
#include"GameObject/Locker/LockerManager.h"
#include"GameObject/Desk/DeskManager.h"
#include"GameObject/Box/BoxManager.h"
#include "Menu/Menu.h"
#include"GameObject/Elevator/ElevatorRoomManager.h"
#include "GameObject/Elevator/Elevator.h"
#include <GameObject/Edamame/Edamame.h>
#include"GameObject/PC/PC.h"
#include "GameObject/Coffee/Coffees.h"
#include "GameObject/UI/DamageOverlay.h"
#include "GameObject/Gentleman/Gentleman.h"
#pragma endregion

#pragma region//Event
#include"GameObject/Event/FirstGameEvent.h"
#pragma endregion

#include"GameObject/UI/UIManager.h"

#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"

#include "Light/CommonLight/DirectionalCommonLight.h" 

#include"Audio.h"
#include <string>
#include "Stages/StageManager.h"

class ShadowGameScene : public BaseScene {
private:
	const float kNoiseTimer_ = 0.5f;
	float noiseTimer_ = kNoiseTimer_;
	bool isNoise_ = false;

	std::unique_ptr<UIManager> uiManager_ = nullptr;

	// カメラの設定
	CameraController* cameraController_ = nullptr;
	std::unique_ptr<StageManager> stageManager_ = nullptr;
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

#pragma region // セーブ
    ProgressSaveData progressSaveData_;
#pragma endregion
#pragma region//ゲームオブジェクトの設定
    //Player
    std::unique_ptr<Player> player_ = nullptr;

    //TestField
    std::unique_ptr<TestField> testField_ = nullptr;
    // ホワイトボード管理
    std::unique_ptr<WhiteBoardManager> whiteBoardManager_ = nullptr;
    //ポータル管理
    std::unique_ptr<PortalManager> portalManager_ = nullptr;
    //携帯打刻機
    std::unique_ptr<TimeCardWatch> timeCardWatch_ = nullptr;
    //懐中電灯
    std::unique_ptr<Flashlight> flashlight_ = nullptr;
    //鍵
    std::unique_ptr<Key> key_ = nullptr;
    //枝豆
    std::unique_ptr<Edamame> edamame_ = nullptr;
    //椅子
    std::unique_ptr<ChairManager> chairManager_ = nullptr;
    //壁管理
    std::unique_ptr<WallManager> wallManager_ = nullptr;
    //壁管理2こめ
    std::unique_ptr<WallManager2> wallManager2_ = nullptr;
    //エレベータ部屋の管理
    std::unique_ptr<ElevatorRoomManager> elevatorRoomManager_ = nullptr;

    //自販機
    std::unique_ptr<VendingMac> vendingMac_ = nullptr;
    //ドア
    std::unique_ptr<Door> door_ = nullptr;
    //ロッカー管理
    std::unique_ptr<LockerManager> lockerManager_ = nullptr;
    //デスク管理
    std::unique_ptr<DeskManager> deskManager_ = nullptr;
    //タイムカード
    std::unique_ptr<TimeCard> timeCard_ = nullptr;
    std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;
    //箱
    std::unique_ptr<BoxManager> boxManager_ = nullptr;
    // エレベーター
    std::unique_ptr<Elevator> elevator_ = nullptr;
    //PC
    std::unique_ptr<PC> pc_ = nullptr;
	// コーヒー缶
	std::unique_ptr<Coffees> coffees_ = nullptr;
    //セーブポイント紳士
    std::unique_ptr<Gentleman> gentleman_ = nullptr;
    //衝突管理
    std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
    //最初のイベント
    std::unique_ptr<FirstGameEvent>firstEvent_ = nullptr;
    GameEvent* currentEvent_ = nullptr;
#pragma region// light
    //DirectionalLight
    DirectionalCommonLight directionalLight_{};

#pragma endregion
    bool useDirectionalShadow_ = true;
    bool usePointShadow_ = false;
    bool useSpotShadow_ = false;
    bool useAreaShadow_ = false;


	std::unique_ptr<DamageOverlay> damageOverlay_ = nullptr;

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
    void DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle, bool drawPlayer,bool drawPlayerHead);
    void SetSceneCameraForDraw(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    void SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle, bool drawPlayer, bool drawPlayerHead);
};

