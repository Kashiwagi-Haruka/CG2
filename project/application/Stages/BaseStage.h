#pragma once
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "Light/CommonLight/AreaCommonLight.h"
#include "Light/CommonLight/PointCommonLight.h"
#include "Light/CommonLight/SpotCommonLight.h"
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
	virtual void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) = 0;
	virtual void UpdatePortal() = 0;
	virtual void CheckCollision(CollisionManager* collisionManager) = 0;
	virtual void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) = 0;
	virtual void SetSceneCameraForDraw(Camera* camera) = 0;
	virtual void SetPlayerCamera(PlayerCamera* playerCamera) = 0;
	virtual PortalManager* GetPortalManager() = 0;
	virtual std::unique_ptr<CollisionManager> GetCollisionManager() = 0;
	virtual PointCommonLight* GetPointLights() = 0;
	virtual uint32_t GetActivePointLightCount() const = 0;
	virtual SpotCommonLight* GetSpotLights() = 0;
	virtual uint32_t GetActiveSpotLightCount() const = 0;
	virtual AreaCommonLight* GetAreaLights() = 0;
	virtual uint32_t GetActiveAreaLightCount() const = 0;
};