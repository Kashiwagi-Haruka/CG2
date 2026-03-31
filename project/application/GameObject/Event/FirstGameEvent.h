#pragma once
#include "GameEvent.h"
class FirstGameEvent :
    public GameEvent
{
private:
    void OnStart()override;
    void OnEnd()override;
};

   
   