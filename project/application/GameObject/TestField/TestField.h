#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"

class Camera;
class TestField
{
public:
    TestField();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
private:
    std::unique_ptr<Primitive>plane_ = nullptr;
    Transform transform_ = {};
};

