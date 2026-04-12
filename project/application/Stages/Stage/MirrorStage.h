#pragma once
#include "Stages/BaseStage.h"
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

public:
	explicit MirrorStage(Player* player);

	void Initialize() override;
	void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) override;
	void UpdatePortal() override;
	void CheckCollision(CollisionManager* collisionManager) override;
	void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) override;
	void SetSceneCameraForDraw(Camera* camera) override;
	void SetPlayerCamera(PlayerCamera* playerCamera) override;
	PortalManager* GetPortalManager() override;
	std::unique_ptr<CollisionManager> GetCollisionManager() override;
};