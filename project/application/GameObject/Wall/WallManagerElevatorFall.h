#pragma once
#include "WallManager.h"
class WallManagerElevatorFall :
    public WallManager

{
public:
    WallManagerElevatorFall();
    void Update()override;
    void Initialize()override;
};
