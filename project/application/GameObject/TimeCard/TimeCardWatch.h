#pragma once
#include"Object3d/Object3d.h"
#include<memory>
#include"Primitive/Primitive.h"
class Player;

class Camera;
class TimeCardWatch
{
public:
    TimeCardWatch();
    void Initialize();
    void SetCamera(Camera* camera);
    void Update();
    void Draw();
    void SetPlayer(Player* player) { player_ = player; }
private:
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    Player* player_ = nullptr;
    Transform transform_ = {};
    Camera* camera_ = nullptr;
};

