#pragma once
#include"Object3d/Object3d.h"
#include<memory>
#include"RigidBody.h"

class WhiteBoard
{
public:
    WhiteBoard();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetModel(const std::string& filePath);
private:
    std::unique_ptr<Object3d>obj_ = nullptr;
    AABB collision_ = {};
};