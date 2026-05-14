#pragma once
#include"Input.h"
#include<memory>
#include"Vector2.h"

enum GameKeyBind
{
    // キーボード
    K_MoveLeft = DIK_A,
    K_MoveRight = DIK_D,
    K_MoveForward = DIK_W,
    K_MoveBackward = DIK_S,

    K_MoveLeftArrow = DIK_LEFT,
    K_MoveRightArrow = DIK_RIGHT,
    K_MoveForwardArrow = DIK_UP,
    K_MoveBackwardArrow = DIK_DOWN,

    K_Sneak = DIK_LSHIFT,
    K_Interact = DIK_E,
    K_Interact2 = DIK_SPACE,


    // コントローラー
    C_MoveLeft = Input::PadButton::kButtonLeft,
    C_MoveRight = Input::PadButton::kButtonRight,
    C_MoveForward = Input::PadButton::kButtonUp,
    C_MoveBackward = Input::PadButton::kButtonDown,
    C_Shot = Input::PadButton::kButtonA,
    C_Sneak = Input::PadButton::kButtonLeftShoulder,
    C_Interact = Input::PadButton::kButtonX,

    //マウス
    M_Shot = Input::MouseButton::kLeft,
    M_Light = Input::MouseButton::kRight,
};

class PlayerCommand {

private:
    static std::unique_ptr<PlayerCommand> instance_;
    // 何かをつかんでいるかどうか
    static bool isGrab_;
    // 何かにたっているかどうか
    static bool isStand_;
    //何かを見ているかどうか
    static bool isLook_;
    // メニュー/オプション表示中など、ゲームプレイ入力を無効化する
    static bool isUiInputLocked_;

public:
    static PlayerCommand* GetInstance();
    PlayerCommand(const PlayerCommand&) = delete;
    PlayerCommand& operator=(const PlayerCommand&) = delete;
    PlayerCommand(PlayerCommand&&) = delete;
    PlayerCommand& operator=(PlayerCommand&&) = delete;
    bool MoveLeft();
    bool MoveRight();
    bool MoveForward();
    bool MoveBackward();
    bool EvetSkipTrigger();
    bool MoveForwardTrigger();
    bool MoveBackwardTrigger();
    bool MenuCommandTrigger();
    bool UiMoveForwardTrigger();
    bool UiMoveBackwardTrigger();
    bool UiMoveLeftTrigger();
    bool UiMoveRightTrigger();
    bool ItemUseTrigger();
    bool SwitchLight();
    bool Shot();
    bool Sneak();
    bool Interact();
    bool InteractTrigger();
    bool UiInteractTrigger();
    bool MouseWheelUp();
    bool MouseWheelDown();
    const float GetMouseWheelDelta();
    bool UiMoveLeft();
    bool UiMoveRight();
    Vector2 Rotate(float rotateSpeed, const bool isFlipHorizontally, const bool isFlipVertically);
    static bool GetIsGrab() { return isGrab_; };
    static void SetIsGrab(bool flag) { isGrab_ = flag; }
    static bool GetIsStand() { return isStand_; };
    static void SetIsStand(bool flag) { isStand_ = flag; }
    static bool GetIsLook() { return isLook_; }
    static void SetIsLook(bool flag) { isLook_ = flag; }
    static bool GetIsUiInputLocked() { return isUiInputLocked_; }
    static void SetIsUiInputLocked(bool flag) { isUiInputLocked_ = flag; }
    static void Initialize();

private:
    bool MoveTrigger(const GameKeyBind key, const GameKeyBind key2, const GameKeyBind controller);
    bool Move(const GameKeyBind key, const GameKeyBind key2, const GameKeyBind controller);
    PlayerCommand() = default;
};