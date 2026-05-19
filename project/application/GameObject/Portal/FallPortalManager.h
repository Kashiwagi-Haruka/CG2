#pragma once
#include"PortalManager.h"

class FallPortalManager :public PortalManager
{
public:
    FallPortalManager(Vector3* pos);
    ~FallPortalManager();
    void Initialize()override;
    void SpawnFirstPortal()override;
    void UpdatePortal()override;
private:
    void SpawnPortal(WhiteBoard* board);
    static bool canMakePortal_;
};

