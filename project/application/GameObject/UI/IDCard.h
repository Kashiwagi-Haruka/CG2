#pragma once
#include<memory>
#include"Sprite.h"

class IDCard
{
public:
    IDCard();
    void Initialize();
    void Update();
    void Draw();
    void InitCount();
private:
    void SetRotate(const float start, const float end,const float lerpTime);
private:

    std::unique_ptr<Sprite> sprite_ = nullptr;
    uint32_t handle_ = 0;
    float rotateTimer_ = 0.0f;
    float lerpTime_ = 0.0f;
    float rotateCoolTimer_ = 0.0f;
    const float kRotateCoolTime_ = 10.0f;
    float rotate_ = 0.0f;
    float startRotate_ = 0.0f;
    float endRotate_ = 0.0f;
    uint32_t rotateCount_ = 0;
    bool isRotateStart = false;
    bool isEaseEnd_ = false;

};

