#pragma once
#include "WallManager.h"


class WallManagerRestRoom : public WallManager
{
public:
    WallManagerRestRoom();
    void Update()override;
    void Initialize()override;
    void Draw()override;
    void SetCamera(Camera* camera)override;
};
