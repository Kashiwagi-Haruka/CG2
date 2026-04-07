#pragma once
#include <memory>
#include "Camera.h"
#include"Transform.h"
#include"RigidBody.h"
#include"GameSave/GameSave.h"

class Player;
class PlayerCamera
{
public:

    PlayerCamera();
    void Update();
    Camera* GetCamera() { return camera_.get(); };
    Transform& GetTransform() { return param_.transform; }
    CameraSaveData& GetParam() { return param_; };
    //Rayをセットする
    void SetRay();
    Ray& GetRay() { return ray_; };
    void Initialize();
    //RayとAABBの当たり判定を共通化しました。
    bool OnCollisionRay(const AABB& localAABB, const Vector3& translate);
    void SetPlayer(Player* player) { player_ = player; }
    void SetHeadTransform();
    void SetTransform();

private:
    //回転
    void Rotate();
    Vector3 GetForward();
private:
    CameraSaveData param_;
    const float rotateSpeed_ = 0.02f;       // カメラの回転速度
    const float kTMin_ = 0.0f;
    const float kTMax_ = 5.0f;

    //カメラからのRay
    Ray ray_;
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    Player* player_ = nullptr;
};

