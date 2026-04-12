#pragma once
#include "BaseStage.h"
#include <memory>
#include <string>

class Player;
class Camera;
class CollisionManager;
class PlayerCamera;
class PortalManager;
struct Vector3;

class StageManager {
private:
	Player* player_ = nullptr;
	std::unique_ptr<BaseStage> stage_ = nullptr;

public:
	explicit StageManager(Player* player);
	void CreateStage(const std::string& sceneName);
	void InitializeStage();
	void UpdateLight();
	void UpdatePlayerDamage();
	void UpdatePostEffect();
	void UpdateGameObject(Camera* camera, const Vector3& lightDirection);
	void UpdatePortal();
	void CheckCollision(CollisionManager* collisionManager);
	void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle);
	void DrawDamageOverlay();
	void SetSceneCameraForDraw(Camera* camera);
	void SetPlayerCamera(PlayerCamera* playerCamera);
	PortalManager* GetPortalManager();
};