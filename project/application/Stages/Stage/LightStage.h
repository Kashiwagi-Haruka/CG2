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
	void DrawSprite()override;
	void SetSceneCameraForDraw(Camera* camera) override;
	void SetPlayerCamera(PlayerCamera* playerCamera) override;
	PortalManager* GetPortalManager() override;

	bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const override;
	void SetLightManager(Yoshida::LightManager* lightManager) { lightManager_ = lightManager; };


private:
	Player* player_ = nullptr;
	Yoshida::LightManager* lightManager_ = nullptr;
};