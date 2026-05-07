#pragma once
#include "WallManager.h"

class WallManagerRoofFoor : public WallManager
{
public:
    WallManagerRoofFoor();
    void Update()override;
    void Initialize()override;
};
