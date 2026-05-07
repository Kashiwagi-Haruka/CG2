#pragma once
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include"GameObject/LightManager/LightManager.h"
#include <memory>
class Camera;
class Player;
class PlayerCamera;
class PortalManager;
struct Vector3;

class BaseStage {
public:
	virtual ~BaseStage() = default;

	virtual void Initialize() = 0;
	virtual void SetPlayer(Player* player) = 0;
	virtual void SetCollisionManager(CollisionManager* collisionManager) = 0;
	virtual void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) = 0;
	virtual void UpdatePortal() = 0;
	virtual void CheckCollision() = 0;
	virtual void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) = 0;
	virtual void DrawSprite() = 0;
	virtual void SetSceneCameraForDraw(Camera* camera) = 0;
	virtual void SetPlayerCamera(PlayerCamera* playerCamera) = 0;
	virtual PortalManager* GetPortalManager() = 0;
	virtual void SetLightManager(Yoshida::LightManager* lightManager) = 0;
	virtual bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const = 0;
};