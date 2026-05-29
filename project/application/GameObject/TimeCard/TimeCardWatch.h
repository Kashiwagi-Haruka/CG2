#pragma once
#include"Object3d/Object3d.h"
#include<memory>
#include"Primitive/Primitive.h"

class Camera;
class TimeCardWatch
{
public:
    TimeCardWatch();
    void Initialize();
    void SetCamera(Camera* camera);
    void Update();
    void Draw();
    void SetParentMat(Matrix4x4* mat) { parentMat_ = mat; }

private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    Matrix4x4* parentMat_ = nullptr;
    Transform transform_ = {};
    Camera* camera_ = nullptr;
};

