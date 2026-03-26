#pragma once
#include"Object3d/Object3d.h"
#include<memory>

class Camera;
class TimeCardRack
{
public:
    TimeCardRack();
    void Initialize();
    void SetCamera(Camera* camera);
    void Update();
    void Draw();
private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    Transform transform_;
};