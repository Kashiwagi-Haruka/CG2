#pragma once
#include"Object3d/Object3d.h"
#include<memory>
#include"RigidBody.h"
#include"Primitive/Primitive.h"

class WhiteBoard
{
public:
    WhiteBoard();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetModel(const std::string& filePath);
    AABB GetAABB();
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
#ifdef _DEBUG
    std::unique_ptr<Primitive>primitive_ = nullptr;
#endif
    AABB localAABB_ = {};
    Transform collisionTransform_ = {};
};