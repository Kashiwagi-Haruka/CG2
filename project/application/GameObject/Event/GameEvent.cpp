#include "GameEvent.h"

void GameEvent::StartEvent()
{
    isRunning_ = true;
    OnStart();
}

void GameEvent::EndEvent()
{
 
    OnEnd();
    isRunning_ = true;
}
