#pragma once
#include"Wall.h"
#include<vector>
#include"Object3d/Object3d.h"
#include"Light/CommonLight/AreaCommonLight.h"
#include"Primitive/Primitive.h"

#include<array>
class WallManager
{
public:
    WallManager();
    ~WallManager();
    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    virtual void SetCamera(Camera* camera);
    std::vector<std::unique_ptr<Wall>>& GetWalls() { return walls_; };
    std::array<AreaCommonLight,2>& GetAreaLights() { return  areaLights_; }
    std::unique_ptr<Object3d>& GetRoom() {
        return room1_;
    };
protected:
    /*std::unique_ptr <Primitive>plane_ = nullptr;*/
    std::array<AreaCommonLight,2> areaLights_;
    Matrix4x4 roomMat_= { 0.0f };
    std::vector<std::unique_ptr<Wall>>walls_;
    std::unique_ptr<Object3d>room1_ = nullptr;
};

