#pragma once
#include "Stages/BaseStage.h"

class LightStage : public BaseStage {
public:
	explicit LightStage(Player* player);

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

private:
	Player* player_ = nullptr;
};