#pragma once
#include"Camera.h"
#include<memory>
#include"Object3d/Object3d.h"
#include"Primitive/Primitive.h"
class WarpPos
{
public:
    WarpPos();
    void SetCamera(Camera* camera);
    void SetTransform(Transform& transform) {transform_ = transform; };
    Vector3& GetTranslate() { return transform_.translate; };
    Vector3& GetRotate() { return transform_.rotate; };
    Transform& GetTransform() { return transform_; }
    void Initialize();
    void Update();
    void Draw();
    Camera* GetWarpPosCamera() { return camera_.get(); };
private:
    std::unique_ptr<Camera> camera_ = nullptr;
    std::unique_ptr<Object3d>object3d_ = nullptr;

    Transform transform_ = {};
    float sinTheta_ = 0.0f;

};

