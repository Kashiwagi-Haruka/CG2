#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"RigidBody.h"
class Camera;
class Portal
{
public:
    Portal();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    Vector3& GetTranslate() { return transform_.translate; };
    const Sphere& GetSphere();
private:
    std::unique_ptr<Primitive>ring_ = nullptr;
    Transform transform_ = {};
    std::unique_ptr<Primitive>sphereMesh_ = nullptr;
    Transform sphereTransform_ = {};
    Matrix4x4 uvMat_ = { 0.0f };
    float uvTranslateY_ = 0.0f ;
    Sphere sphere_ = { 0.0f };
};

