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
    void SetRay(const Vector3& origin,const Vector3& diff);
    void SetTransformPtr(Transform* transform) { parentTransform_ = transform; };
    bool OnCollisionObjOfMakePortal(const AABB& aabb, const Transform& transform);
    void MakeBillboardWorldMat();
private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    Ray ray_;
    std::unique_ptr<Primitive> ring_ = nullptr;
    Transform* parentTransform_ = nullptr;
    Transform transform_ = {};
    Transform lineTransform_ = {};
    Camera* camera_ = nullptr;
};

