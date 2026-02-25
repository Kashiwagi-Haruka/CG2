#pragma once
#include <memory>
#include "Camera.h"
#include"Transform.h"

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
private:
    void SetTransform();
    Transform cameraTransform_ = {};
    //カメラの設定
    std::unique_ptr<Camera> camera_ = nullptr;
    Transform* playerTransform_ = nullptr;
    //カメラの感度
    float eyeRotateSpeed_ = 0.3f;
};

