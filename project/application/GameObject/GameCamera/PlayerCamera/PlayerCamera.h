#pragma once
#include <memory>
#include "Camera.h"
#include"Transform.h"
#include"RigidBody.h"

class PlayerCamera
{
public:
    PlayerCamera();
    void Update();
    Camera* GetCamera() { return camera_.get(); };
    Transform& GetTransform() {  return cameraTransform_; }
    void SetPlayerTransformPtr(Transform* transformPtr) {
        playerTransform_ = transformPtr ;
    }
    //回転
    void Rotate();
    //Rayをセットする
    void SetRay();
    Ray& GetRay() { return ray_; };
private:
    void SetTransform();
    Vector3 GetForward();
    Transform cameraTransform_ = {};
    //カメラからのRay
    Ray ray_;
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    Transform* playerTransform_ = nullptr;
    //カメラの感度
    float eyeRotateSpeed_ = 0.3f;
};

