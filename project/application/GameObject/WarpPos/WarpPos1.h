#pragma once
#include"Camera.h"
#include<memory>
#include"Object3d/Object3d.h"
#include"Primitive/Primitive.h"
class WarpPos1
{
public:
    WarpPos1();
    void Initialize();
    void SetCamera(Camera* camera);
    void Update();
    void Draw();
    Vector3& GetTranslate() { return transform_.translate; };
private:
    std::unique_ptr<Camera> camera_ = nullptr;
    std::unique_ptr<Object3d>object3d_ = nullptr;

    Transform transform_ = {};
    float sinTheta_ = 0.0f;

};

