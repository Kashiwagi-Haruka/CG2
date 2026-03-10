#pragma once
#include"Wall.h"
#include<vector>

class WallManager
{
public:
    WallManager();
    ~WallManager();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    std::vector<std::unique_ptr<Wall>>& GetWalls() { return walls_; };
private:
    std::vector<std::unique_ptr<Wall>>walls_;
};

