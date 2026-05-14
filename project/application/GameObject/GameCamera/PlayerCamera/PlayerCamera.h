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
    void SetParam(const CameraSaveData& param) { param_ = param; };
    //Rayをセットする
    void SetRay();
    Ray& GetRay() { return ray_; };
    void Initialize();
    //RayとAABBの当たり判定を共通化しました。
    bool OnCollisionRay(const AABB& localAABB, const Vector3& translate,const float min = 0.0f,const  float max = 5.0f);
    void SetPlayer(Player* player) { player_ = player; }
    void SetHeadTransform();
    void SetTransform();
    void EnableFixedTransform(const Transform& transform);
    void DisableFixedTransform();
    bool IsFixedTransformEnabled() const { return isFixedTransformEnabled_; }

private:
    //回転
    void Rotate();
    Vector3 GetForward();
private:
    CameraSaveData param_;
    const float rotateSpeed_ = 0.02f;       // カメラの回転速度

    //カメラからのRay
    Ray ray_;
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    Player* player_ = nullptr;
    bool isFixedTransformEnabled_ = false;
    Transform fixedTransform_{};
};
