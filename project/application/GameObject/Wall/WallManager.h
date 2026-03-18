#pragma once
#include"Wall.h"
#include<vector>
#include"Object3d/Object3d.h"
#include"Light/AreaLight.h"
#include"Primitive/Primitive.h"

#include<array>
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
    AreaLight& GetAreaLight() { return  areaLight_; }
private:
    std::unique_ptr <Primitive>plane_ = nullptr;
    AreaLight areaLight_;
    Matrix4x4 roomMat_= { 0.0f };
    std::vector<std::unique_ptr<Wall>>walls_;
    std::unique_ptr<Object3d>room1_ = nullptr;
};

