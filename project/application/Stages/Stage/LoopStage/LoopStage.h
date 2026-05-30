#pragma once
#include"../../BaseStage.h"
#include<map>
class Flashlight;
class TestField;
class ObjectCollider;
class LoopStage   : public BaseStage
{
    Player* player_ = nullptr;
    Yoshida::LightManager* lightManager_ = nullptr;
    std::unique_ptr<PortalManager> portalManager_ = nullptr;
    CollisionManager* stageCollisionManager_ = nullptr;
    std::unique_ptr<Flashlight> flashlight_ = nullptr;
    std::map< std::string, std::unique_ptr<ObjectCollider>>fieldCollider_;
private:
  void InitializeLights();
  void UpdateLights();

public:
    explicit LoopStage(Player* player);
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
    void SetLightManager(Yoshida::LightManager* lightManager) override;
    bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed)const override { return false; }
};

