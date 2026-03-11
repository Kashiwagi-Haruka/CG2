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
    static bool& GetCanMakePortal() { return canMakePortal_; };
    void SetTransformPtr(Transform* transform) { parentTransform_ = transform; };
    bool OnCollisionObjOfMakePortal(const Ray& ray,const AABB& aabb, const Transform& transform);
private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    Transform* parentTransform_ = nullptr;
    Transform transform_ = {};
    Camera* camera_ = nullptr;
    static bool canMakePortal_;
};

