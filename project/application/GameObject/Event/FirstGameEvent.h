#pragma once
#include "GameEvent.h"
#include<stdint.h>
#include"Vector3.h"

class FirstGameEvent :
    public GameEvent
{
public:

private:
    void OnStart()override;
    void OnEnd()override;
    void Update()override;
private:
    uint32_t triggerCount_ = 0;
    uint32_t count_ = 0;
    Vector3 cameraForward_;
};

   
   