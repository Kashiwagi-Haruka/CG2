#pragma once
#include "../../BaseStage.h"
class TestField;
class Door;
class Key;
class WallManager;
class WallManager2;
class WhiteBoardManager;
class TimeCard;
class TimeCardRack;
class TimeCardWatch;

class TutorialStage :
    public BaseStage
{
    Player* player_ = nullptr;
    Yoshida::LightManager* lightManager_ = nullptr;
    std::unique_ptr<TestField> testField_ = nullptr;
    std::unique_ptr<Key> key_ = nullptr;
    std::unique_ptr<Door> door_ = nullptr;

    std::unique_ptr<TimeCard> timeCard_ = nullptr;
    std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;

    std::unique_ptr<WhiteBoardManager>  whiteBoardManager_ = nullptr;
    std::unique_ptr<PortalManager> portalManager_ = nullptr;
    CollisionManager* stageCollisionManager_ = nullptr;

    std::unique_ptr<WallManager> wallManager_ = nullptr;
    std::unique_ptr<WallManager2> wallManager2_ = nullptr;

private:
    void InitializeLights();
public:
    explicit TutorialStage(Player* player);
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
};

