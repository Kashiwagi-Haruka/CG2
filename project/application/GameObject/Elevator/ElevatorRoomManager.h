#pragma once
#include"GameObject/Wall/Wall.h"
#include<vector>
#include"Object3d/Object3d.h"
#include"Light/CommonLight/AreaCommonLight.h"
#include"Primitive/Primitive.h"

#include<array>
class ElevatorRoomManager
{
public:
    ElevatorRoomManager();
    ~ElevatorRoomManager();
    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    void SetCamera(Camera* camera);
    std::vector<std::unique_ptr<Wall>>& GetWalls() { return walls_; };
    AreaCommonLight& GetAreaLight() { return  areaLight_; }
protected:
    AreaCommonLight areaLight_;
    Matrix4x4 roomMat_ = { 0.0f };
    std::vector<std::unique_ptr<Wall>>walls_;
    std::unique_ptr<Object3d>room_ = nullptr;
};

