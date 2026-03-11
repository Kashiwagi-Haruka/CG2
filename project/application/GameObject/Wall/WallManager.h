#pragma once
#include"Wall.h"
#include<vector>
#include"Object3d/Object3d.h"

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
    std::unique_ptr<Object3d>room1_ = nullptr;
};

