#pragma once
#include"Object3d/Object3d.h"
#include<memory>
#include"RigidBody.h"
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

    void SetTransformPtr(Transform* transform) { parentTransform_ = transform; };
    bool OnCollisionObjOfMakePortal(const Ray& ray,const AABB& aabb, const Transform& transform);
    void MakeWorldMat();
    void MakeBillboardWorldMat();
private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    std::unique_ptr<Primitive> ring_ = nullptr;
    Transform* parentTransform_ = nullptr;
    Transform transform_ = {};
    Transform ringTransform_ = {};
    Camera* camera_ = nullptr;
    Matrix4x4 ringMatWorld_;
    bool canMakePortal_ = false;
};

