#pragma once
#include "WallManager.h"
class WallManager2 :
    public WallManager

{
public:
    WallManager2();
    void Update()override;
    void Initialize()override;
};
