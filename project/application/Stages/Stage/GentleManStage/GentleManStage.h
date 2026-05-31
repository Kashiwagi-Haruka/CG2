#pragma once
#include"../../BaseStage.h"

class Door;
class Key;
class WallManagerRoofFloor;

class TimeCardWatch;
class DocumentManager;
class GentlemanPortalManager;
class GiantGentleMan;
class RotatingPlaygroundEquipment;

class Flashlight;

class GentleManStage :
    public BaseStage
{
    Player* player_ = nullptr;
    Yoshida::LightManager* lightManager_ = nullptr;

    std::unique_ptr<GentlemanPortalManager> portalManager_ = nullptr;
    CollisionManager* stageCollisionManager_ = nullptr;
    std::unique_ptr<WallManagerRoofFloor> wallManagerRoofFloor_ = nullptr;
    std::unique_ptr<DocumentManager>documentManager_ = nullptr;
    std::unique_ptr<GiantGentleMan>giantGentleMan_ = nullptr;
    std::unique_ptr<RotatingPlaygroundEquipment> rotatingPlaygroundEquipment_ = nullptr;
    std::unique_ptr<Flashlight> flashlight_ = nullptr;
    std::unique_ptr<Door> door_ = nullptr;
private:
	void InitializeLights();
	void UpdateLights();

public:
	explicit GentleManStage(Player* player);
	void Initialize() override;
	void SetPlayer(Player* player) override;
	void SetCollisionManager(CollisionManager* collisionManager) override;
	void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) override;
	void UpdatePortal() override;
	void CheckCollision() override;
	void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) override;
	void DrawSprite() override;
	void SetSceneCameraForDraw(Camera* camera) override;
	void SetPlayerCamera(PlayerCamera* playerCamera) override;
	PortalManager* GetPortalManager() override;
	void SetLightManager(Yoshida::LightManager* lightManager) override;
	bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const override { return false; }
};