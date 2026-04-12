#pragma once

class Camera;
class CollisionManager;
class Player;
class PlayerCamera;
class PortalManager;
struct Vector3;

class BaseStage {
public:
	virtual ~BaseStage() = default;

	virtual void Initialize() = 0;
	virtual void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) = 0;
	virtual void UpdatePortal() = 0;
	virtual void CheckCollision(CollisionManager* collisionManager) = 0;
	virtual void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) = 0;
	virtual void SetSceneCameraForDraw(Camera* camera) = 0;
	virtual void SetPlayerCamera(PlayerCamera* playerCamera) = 0;
	virtual PortalManager* GetPortalManager() = 0;
};