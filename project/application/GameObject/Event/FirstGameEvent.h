#pragma once
#include "GameEvent.h"
#include<stdint.h>
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
};

   
   