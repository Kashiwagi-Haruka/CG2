#include "FirstGameEvent.h"
#include"GameBase.h"
#include"GameObject/KeyBindConfig.h"

void FirstGameEvent::Update()
{
    if (IsRunning()) {
        //走っているとき
        timer_ -= GameBase::GetInstance()->GetDeltaTime();

        if (timer_ <= 0.0f||
            PlayerCommand::GetInstance()->InteractTrigger()||
            PlayerCommand::GetInstance()->Shot()) {
            EndEvent();
        }
    }
}

void FirstGameEvent::OnStart()
{
    timer_ = kTime_;
}

void FirstGameEvent::OnEnd()
{

}
