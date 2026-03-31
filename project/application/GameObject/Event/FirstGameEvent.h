#pragma once
#include "GameEvent.h"
class FirstGameEvent :
    public GameEvent
{
public:
    void Update();
private:
    void OnStart()override;
    void OnEnd()override;
private:
    const float kTime_ = 10.0f;
    float timer_ = kTime_;
};

   
   