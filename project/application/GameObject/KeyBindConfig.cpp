#include"KeyBindConfig.h"

std::unique_ptr<PlayerCommand> PlayerCommand::instance_ = nullptr;

PlayerCommand* PlayerCommand::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = std::unique_ptr<PlayerCommand>(new PlayerCommand());
    }
    return instance_.get();
}

bool PlayerCommand::MoveLeft()
{
    return Move(K_MoveLeft, C_MoveLeft);
}

bool PlayerCommand::MoveRight()
{
    return Move(K_MoveRight, C_MoveRight);
}

bool PlayerCommand::MoveForward()
{
    return  Move(K_MoveForward, C_MoveForward);
}

bool PlayerCommand::MoveBackward()
{
    return  Move(K_MoveBackward, C_MoveBackward);
}

bool PlayerCommand::Shot()
{
    auto* input = Input::GetInstance();
    return input->TriggerKey(K_Shot) || input->TriggerButton(Input::PadButton(C_Shot));
}

bool PlayerCommand::Sneak()
{
    auto* input = Input::GetInstance();
    return input->PushKey(K_Sneak) || input->PushButton(Input::PadButton(C_Sneak));
}

bool PlayerCommand::Move(const GameKeyBind key, const GameKeyBind controller)
{
    auto* input = Input::GetInstance();
    return input->PushKey(key) || input->PushButton(Input::PadButton(controller));
}
