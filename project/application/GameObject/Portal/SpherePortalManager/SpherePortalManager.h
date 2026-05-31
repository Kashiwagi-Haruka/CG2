#pragma once
#include"../PortalManager.h"
class GiantEnemyManager;
class GiantEnemy;

class SpherePortalManager :public PortalManager
{
public:
    SpherePortalManager(Vector3* pos);
    ~SpherePortalManager();
    void Initialize()override;
    void UpdatePortal()override;
    void CheckCollision(const bool isOneSide = true)override;
    void SetGiantEnemyManager(GiantEnemyManager* giantEnemyManager);
private:
    void SpawnPortal(GiantEnemy* gianeEnemy);
    GiantEnemyManager* giantEnemyManager_ = nullptr;
    GiantEnemy* pendingGiantEnemy_ = nullptr;
};

