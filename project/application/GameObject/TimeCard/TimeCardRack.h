#pragma once
#include"Object3d/Object3d.h"
#include<memory>

class Camera;
class TimeCardRack
{
public:
    TimeCardRack();
    void Initialize();
    void SetCamera(Camera* camera);
    void Update();
    void Draw();
    void SetTranslate(const Transform& transform) { modelObj_->SetTransform(transform); }
private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;

};