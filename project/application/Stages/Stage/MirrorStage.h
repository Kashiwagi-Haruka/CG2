#pragma once

#include "Stages/BaseStage.h"
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

#include <GameObject/Edamame/Edamame.h>
#include"GameObject/PC/PC.h"

#include "GameObject/Coffee/Coffees.h"



#pragma region // Event
#include "GameObject/Event/FirstGameEvent.h"
#pragma endregion
#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"

class Player;
class ElevatorRoomManager;

class MirrorStage : public BaseStage {

	   // TestField
	std::unique_ptr<TestField> testField_ = nullptr;
	// ホワイトボード管理
	std::unique_ptr<WhiteBoardManager> whiteBoardManager_ = nullptr;
	// ポータル管理
	std::unique_ptr<PortalManager> portalManager_ = nullptr;
	// 携帯打刻機
	std::unique_ptr<TimeCardWatch> timeCardWatch_ = nullptr;
	// 懐中電灯
	std::unique_ptr<Flashlight> flashlight_ = nullptr;
	// 鍵
	std::unique_ptr<Key> key_ = nullptr;
	// 枝豆
	std::unique_ptr<Edamame> edamame_ = nullptr;
	// 椅子
	std::unique_ptr<ChairManager> chairManager_ = nullptr;
	// 壁管理
	std::unique_ptr<WallManager> wallManager_ = nullptr;
	// 壁管理2こめ
	std::unique_ptr<WallManager2> wallManager2_ = nullptr;

	    // 自販機
	std::unique_ptr<VendingMac> vendingMac_ = nullptr;
	// ドア
	std::unique_ptr<Door> door_ = nullptr;
	// ロッカー管理
	std::unique_ptr<LockerManager> lockerManager_ = nullptr;
	// デスク管理
	std::unique_ptr<DeskManager> deskManager_ = nullptr;
	// タイムカード
	std::unique_ptr<TimeCard> timeCard_ = nullptr;
	std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;
	// 箱
	std::unique_ptr<BoxManager> boxManager_ = nullptr;

	   // PC
	std::unique_ptr<PC> pc_ = nullptr;
	// コーヒー缶
	std::unique_ptr<Coffees> coffees_ = nullptr;

	// 衝突管理
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	// 最初のイベント
	std::unique_ptr<FirstGameEvent> firstEvent_ = nullptr;
	GameEvent* currentEvent_ = nullptr;

	Player* player_ = nullptr;
	ElevatorRoomManager* elevatorRoomManager_ = nullptr;
	// PointLight
	std::array<PointCommonLight, kMaxPointLights> pointLights_{};
	uint32_t activePointLightCount_ = 0;
	// SpotLight
	std::array<SpotCommonLight, kMaxSpotLights> spotLights_{};
	uint32_t activeSpotLightCount_ = 0;
	// AreaLight
	std::array<AreaCommonLight, kMaxAreaLights> areaLights_{};
	uint32_t activeAreaLightCount_ = 0;

	const float kNoiseTimer_ = 0.5f;
	float noiseTimer_ = kNoiseTimer_;
	bool isNoise_ = false;
	bool usePointShadow_ = false;
	bool useSpotShadow_ = false;
	bool useAreaShadow_ = false;
	float playerHp_ = 3.0f;
	static constexpr float kPlayerMaxHp_ = 3.0f;
	float damageCooldownTimer_ = 0.0f;
	bool didTakeDamage_ = false;
	PlayerCamera* playerCamera_ = nullptr;

public:
	MirrorStage();
	void Initialize() override;
	void Update() override;
	void ShadowMapDraw() override;
	void MainDraw() override;
	void Finalize() override;
	void SetPlayer(Player* player) override;
	void SetPlayerCamera(PlayerCamera* playerCamera) override;
	void SetElevatorManager(ElevatorRoomManager* elevatorRoomManager) override;
	bool IsCurrentEventRunning() const override;
	float GetPlayerHp() const override { return playerHp_; }
	float GetPlayerMaxHp() const override { return kPlayerMaxHp_; }
	bool DidPlayerTakeDamage() const override { return didTakeDamage_; }
	bool IsPlayerDead() const override { return playerHp_ <= 0.0f; }

private:
	void UpdatePlayerDamage();
	void ApplyPlayerDamage(float damageAmount);
	void CheckCollision();
	void DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle);
	void SetSceneCameraForDraw(Camera* camera);
	void SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle);
	void UpdateLight();
	void UpdatePostEffect();
	void InitializeLights();
};