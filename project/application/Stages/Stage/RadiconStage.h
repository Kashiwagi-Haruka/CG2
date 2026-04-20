#pragma once
#include <memory>
#include "Stages/BaseStage.h"
class RadiconStage : public BaseStage {

	void Initialize() override;
	void SetPlayer(Player* player)override;
	void SetCollisionManager(CollisionManager* collisionManager)override;
	void UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player)override;
	void UpdatePortal()override;
	void CheckCollision()override;
	void DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle)override;
	void SetSceneCameraForDraw(Camera* camera)override;
	void SetPlayerCamera(PlayerCamera* playerCamera)override;
	PortalManager* GetPortalManager()override;
	std::unique_ptr<CollisionManager> GetCollisionManager()override;
	void SetLightManager(Yoshida::LightManager* lightManager)override;
	bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const override;

};
