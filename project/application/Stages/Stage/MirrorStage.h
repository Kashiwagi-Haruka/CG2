#pragma once
#include "Light/CommonLight/AreaCommonLight.h"
#include "Light/CommonLight/PointCommonLight.h"
#include "Light/CommonLight/SpotCommonLight.h"
#include "Stages/BaseStage.h"
#include <array>
#include <memory>

class BoxManager;
class ChairManager;
class Coffees;
class DeskManager;
class Door;
class Edamame;
class Flashlight;
class Key;
class LockerManager;
class PC;
class Player;
class PortalManager;
class TestField;
class TimeCard;
class TimeCardRack;
class TimeCardWatch;
class VendingMac;
class WallManager;
class WallManager2;
class WhiteBoardManager;
class CollisionManager;

class MirrorStage : public BaseStage {
private:
	Player* player_ = nullptr;

	std::unique_ptr<TestField> testField_ = nullptr;
	std::unique_ptr<WhiteBoardManager> whiteBoardManager_ = nullptr;
	std::unique_ptr<PortalManager> portalManager_ = nullptr;
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	CollisionManager* stageCollisionManager_ = nullptr;
	std::unique_ptr<PC> pc_ = nullptr;
	std::unique_ptr<Coffees> coffees_ = nullptr;
	std::unique_ptr<TimeCardWatch> timeCardWatch_ = nullptr;
	std::unique_ptr<Flashlight> flashlight_ = nullptr;
	std::unique_ptr<Key> key_ = nullptr;
	std::unique_ptr<Edamame> edamame_ = nullptr;
	std::unique_ptr<Door> door_ = nullptr;
	std::unique_ptr<LockerManager> lockerManager_ = nullptr;
	std::unique_ptr<WallManager> wallManager_ = nullptr;
	std::unique_ptr<WallManager2> wallManager2_ = nullptr;
	std::unique_ptr<VendingMac> vendingMac_ = nullptr;
	std::unique_ptr<ChairManager> chairManager_ = nullptr;
	std::unique_ptr<DeskManager> deskManager_ = nullptr;
	std::unique_ptr<TimeCard> timeCard_ = nullptr;
	std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;
	std::unique_ptr<BoxManager> boxManager_ = nullptr;

	std::array<PointCommonLight, kMaxPointLights> pointLights_{};
	uint32_t activePointLightCount_ = 0;
	std::array<SpotCommonLight, kMaxSpotLights> spotLights_{};
	uint32_t activeSpotLightCount_ = 0;
	std::array<AreaCommonLight, kMaxAreaLights> areaLights_{};
	uint32_t activeAreaLightCount_ = 0;

	void InitializeLights();
	void UpdateLights();

public:
	explicit MirrorStage(Player* player);

	void Initialize() override;
	void SetPlayer(Player* player) override;
	void SetCollisionManager(CollisionManager* collisionManager) override;
	void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) override;
	void UpdatePortal() override;
	void CheckCollision() override;
	void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) override;
	void SetSceneCameraForDraw(Camera* camera) override;
	void SetPlayerCamera(PlayerCamera* playerCamera) override;
	PortalManager* GetPortalManager() override;
	std::unique_ptr<CollisionManager> GetCollisionManager() override;
	PointCommonLight* GetPointLights() override;
	uint32_t GetActivePointLightCount() const override;
	SpotCommonLight* GetSpotLights() override;
	uint32_t GetActiveSpotLightCount() const override;
	AreaCommonLight* GetAreaLights() override;
	uint32_t GetActiveAreaLightCount() const override;
};