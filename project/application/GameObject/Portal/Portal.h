#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"

class Camera;
class Portal
{
public:
    Portal();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
private:
    std::unique_ptr<Primitive>ring_ = nullptr;
    Transform transform_ = {};
    Matrix4x4 uvMat_ = { 0.0f };
    float uvTranslateY_ = 0.0f ;
};

