#pragma once
#include"../../BaseStage.h"
class TestField;
class Door;
class Key;
class WallManagerRoofFoor;
class WhiteBoardManager;
class TimeCardWatch;
class DocumentManager;

class GentleManStage :
    public BaseStage
{
    Player* player_ = nullptr;
    Yoshida::LightManager* lightManager_ = nullptr;
    std::unique_ptr<TestField> testField_ = nullptr;

    std::unique_ptr<TimeCardWatch> timeCardWatch_ = nullptr;
    std::unique_ptr<WhiteBoardManager>  whiteBoardManager_ = nullptr;
    std::unique_ptr<PortalManager> portalManager_ = nullptr;
    CollisionManager* stageCollisionManager_ = nullptr;
    std::unique_ptr<WallManagerRoofFoor> wallManagerRoofFoor_ = nullptr;
    std::unique_ptr<DocumentManager>documentManager_ = nullptr;

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
    void DrawSprite()override;
    void SetSceneCameraForDraw(Camera* camera) override;
    void SetPlayerCamera(PlayerCamera* playerCamera) override;
    PortalManager* GetPortalManager() override;
    void SetLightManager(Yoshida::LightManager* lightManager) override;
    bool CheckHitPlayerByStageHazard(const Vector3& playerPosition, float playerRadius, float minHitSpeed)const override { return false; }
};

