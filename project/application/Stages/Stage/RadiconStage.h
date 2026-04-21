#pragma once
#include "Stages/BaseStage.h"
#include <memory>
#include "GameObject/Radicon/Radicon.h"
#include "GameObject/Enemy/Enemy.h"
#include "GameObject/BurningObject/BurningObject.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "GameObject/TestField/TestField.h"
class RadiconStage : public BaseStage {
public:
	explicit RadiconStage(Player* player);

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
	void SetLightManager(Yoshida::LightManager* lightManager) override;
	bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const override;

private:
	Player* player_ = nullptr;
	Yoshida::LightManager* lightManager_ = nullptr;

	std::unique_ptr<Radicon> radicon_;
	std::unique_ptr<Enemy> enemy_;
	std::unique_ptr<BurningObject> burningObject_;
	std::unique_ptr<TestField> testField_;
};