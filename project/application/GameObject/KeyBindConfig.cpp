#include "KeyBindConfig.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"

std::unique_ptr<PlayerCommand> PlayerCommand::instance_ = nullptr;
bool PlayerCommand::isGrab_ = false;
bool PlayerCommand::isStand_ = false;
bool PlayerCommand::isUiInputLocked_ = false;

PlayerCommand* PlayerCommand::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = std::unique_ptr<PlayerCommand>(new PlayerCommand());
	}
	return instance_.get();
}

bool PlayerCommand::MoveLeft() {
	if (isUiInputLocked_) {
		return false;
	}
	return Move(K_MoveLeft, K_MoveLeftArrow, C_MoveLeft);
}

bool PlayerCommand::MoveRight() {
	if (isUiInputLocked_) {
		return false;
	}
	return Move(K_MoveRight, K_MoveRightArrow, C_MoveRight);
}

bool PlayerCommand::MoveForward() {
	if (isUiInputLocked_) {
		return false;
	}
	return Move(K_MoveForward, K_MoveForwardArrow, C_MoveForward);
}

bool PlayerCommand::MoveBackward() {
	if (isUiInputLocked_) {
		return false;
	}
	return Move(K_MoveBackward, K_MoveBackwardArrow, C_MoveBackward);
}

bool PlayerCommand::MoveForwardTrigger() {
	if (isUiInputLocked_) {
		return false;
	}
	return MoveTrigger(K_MoveForward, K_MoveForwardArrow, C_MoveForward);
}

bool PlayerCommand::MoveBackwardTrigger() {
	if (isUiInputLocked_) {
		return false;
	}
	return MoveTrigger(K_MoveBackward, K_MoveBackwardArrow, C_MoveBackward);
}

bool PlayerCommand::Shot() {
	auto* input = Input::GetInstance();
	return input->TriggerKey(K_Shot) || input->TriggerButton(Input::PadButton(C_Shot)) || input->TriggerMouseButton(Input::MouseButton(M_Shot));
}

bool PlayerCommand::Sneak() {
	auto* input = Input::GetInstance();
	return input->PushKey(K_Sneak) || input->PushButton(Input::PadButton(C_Sneak));
}

bool PlayerCommand::Interact() {
	if (isUiInputLocked_) {
		return false;
	}
	auto* input = Input::GetInstance();
	return input->PushKey(K_Interact) || input->PushButton(Input::PadButton(C_Interact));
}

bool PlayerCommand::InteractTrigger() {
	if (isUiInputLocked_) {
		return false;
	}
	auto* input = Input::GetInstance();
	return input->TriggerKey(K_Interact) || input->TriggerButton(Input::PadButton(C_Interact));
}

bool PlayerCommand::MouseWheelUp() {
	auto* input = Input::GetInstance();
	return (input->GetMouseWheelDelta() < 0.0f);
}

bool PlayerCommand::MouseWheelDown() {
	auto* input = Input::GetInstance();
	return (input->GetMouseWheelDelta() > 0.0f);
}

Vector2 PlayerCommand::Rotate(float rotateSpeed) {
	auto* input = Input::GetInstance();

	Vector2 inputMovePos = input->GetJoyStickRXY();
	float dPitch = 0.0f;
	float dYaw = 0.0f;

	if (fabs(inputMovePos.x) > 0.0f || fabs(inputMovePos.y) > 0.0f) {
		// スティック処理が優先される
		dYaw = inputMovePos.x * YoshidaMath::kDeltaTime * rotateSpeed * 10.0f;
		dPitch = -inputMovePos.y * YoshidaMath::kDeltaTime * rotateSpeed * 10.0f;
	} else {
		// マウス
		inputMovePos = input->GetMouseMove();
		dYaw += inputMovePos.x * YoshidaMath::kDeltaTime * rotateSpeed;
		dPitch += inputMovePos.y * YoshidaMath::kDeltaTime * rotateSpeed;
	}

	return {dPitch, dYaw};
}

void PlayerCommand::Initialize() {
	isGrab_ = false;
	// 何かにたっているかどうか
	isStand_ = false;
	isUiInputLocked_ = false;
}

bool PlayerCommand::MoveTrigger(const GameKeyBind key, const GameKeyBind key2, const GameKeyBind controller) {
	auto* input = Input::GetInstance();
	return input->TriggerKey(key) || input->TriggerKey(key2) || input->TriggerButton(Input::PadButton(controller));
}

bool PlayerCommand::Move(const GameKeyBind key, const GameKeyBind key2, const GameKeyBind controller) {
	auto* input = Input::GetInstance();
	return input->PushKey(key) || input->PushKey(key2) || input->PushButton(Input::PadButton(controller));
}