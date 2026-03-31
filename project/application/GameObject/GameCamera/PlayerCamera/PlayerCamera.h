#pragma once
#include <memory>
#include "Camera.h"
#include"Transform.h"
#include"RigidBody.h"

class Player;
class PlayerCamera
{
public:
    PlayerCamera();
    void Update();
    Camera* GetCamera() { return camera_.get(); };
    Transform& GetTransform() { return cameraTransform_; }

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
    const float kTMin_ = 0.0f;
    const float kTMax_ = 5.0f;
    //回転
    void Rotate();
    Vector3 GetForward();
    Transform cameraTransform_ = {};
    //カメラからのRay
    Ray ray_;
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    //カメラの感度
    float eyeRotateSpeed_ = 0.02f;

    Player* player_ = nullptr;
};

