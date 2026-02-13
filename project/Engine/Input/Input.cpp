#include "Input.h"
#include <cassert>
#include <cmath>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;
std::unique_ptr<Input> Input::instance_ = nullptr;

Input* Input::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = std::unique_ptr<Input>(new Input());
	}
	return instance_.get();
}
BOOL CALLBACK Input::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
	Input* input = reinterpret_cast<Input*>(pContext);
	HRESULT hr = input->directInput->CreateDevice(pdidInstance->guidInstance, &input->gamePadDevice_, NULL);
	if (FAILED(hr)) {
		return DIENUM_CONTINUE;
	}
	return DIENUM_STOP;
}

void Input::Initialize(WinApp* winApp) {

	winApp_ = winApp;

	HRESULT result;

	// DirectInput 作成
	result = DirectInput8Create(winApp_->GetHinstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボード
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = keyboard->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// ゲームパッド列挙 → 最初の1台を使用
	directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);

	if (gamePadDevice_) {
		result = gamePadDevice_->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(result));
		result = gamePadDevice_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
		assert(SUCCEEDED(result));
	}

	result = directInput->CreateDevice(GUID_SysMouse, &mouseDevice_, nullptr);
	assert(SUCCEEDED(result));

	result = mouseDevice_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	result = mouseDevice_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::SetIsCursorVisible(bool isVisible) { isCursorVisibleRequested_ = isVisible; }
void Input::Update() {
	if (isCursorVisible_ != isCursorVisibleRequested_) {
		if (isCursorVisibleRequested_) {
			while (ShowCursor(TRUE) < 0) {
			}
		} else {
			while (ShowCursor(FALSE) >= 0) {
			}
		}
		isCursorVisible_ = isCursorVisibleRequested_;
	}
	// 前のフレームのキー入力を保存
	memcpy(preKey, key, sizeof(key));

	// キーボードの状態を取得
	keyboard->Acquire();
	keyboard->GetDeviceState(sizeof(key), key);

	// ゲームパッド
	prePadState_ = padState_;

	if (gamePadDevice_) {
		HRESULT hr = gamePadDevice_->Acquire();
		if (SUCCEEDED(hr)) {
			hr = gamePadDevice_->GetDeviceState(sizeof(DIJOYSTATE), &padState_);
			if (FAILED(hr)) {
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
					// 再度 Acquire を試みる
					gamePadDevice_->Acquire();
				} else {
					// 本当にダメな場合だけリセット
					gamePadDevice_.Reset();
				}
			}
		} else {
			if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
				// 未取得なら再度 Acquire を試みる
				gamePadDevice_->Acquire();
			} else {
				// それ以外の致命的なエラーならリセット
				gamePadDevice_.Reset();
			}
		}
	}

	if (winApp_->GetIsPad()) {

		// --- もしデバイスが切れていたら再列挙 ---
		if (!gamePadDevice_) {
			directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
			if (gamePadDevice_) {
				gamePadDevice_->SetDataFormat(&c_dfDIJoystick);
				gamePadDevice_->SetCooperativeLevel(GetActiveWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			}
		}
		winApp_->SetIsPad(false);
	}

	prevMouseState_ = mouseState_;

	// ２）DirectInput で相対移動だけ取得（マウスホイールやボタンは要るなら使う）
	if (mouseDevice_) {
		mouseDevice_->Acquire();
		mouseDevice_->GetDeviceState(sizeof(mouseState_), &mouseState_);
	}

	// ３）Windows API でマウスの絶対位置を取得
	POINT pt;
	GetCursorPos(&pt);                       // スクリーン座標
	ScreenToClient(winApp_->GetHwnd(), &pt); // クライアント座標に変換
	mouseX_ = pt.x;                          // ここではクランプせずそのまま代入
	mouseY_ = pt.y;
	if (isCursorStability) {
		if (GetForegroundWindow() == winApp_->GetHwnd()) {
			RECT clientRect{};
			if (GetClientRect(winApp_->GetHwnd(), &clientRect)) {
				POINT center{(clientRect.right - clientRect.left) / 2, (clientRect.bottom - clientRect.top) / 2};
				POINT screenCenter = center;
				ClientToScreen(winApp_->GetHwnd(), &screenCenter);
				SetCursorPos(screenCenter.x, screenCenter.y);
				mouseX_ = center.x;
				mouseY_ = center.y;
			}
		}
	}
}

bool Input::PushKey(BYTE keyNumber) {

	if (key[keyNumber]) {

		return true;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber) {

	if (key[keyNumber] && !preKey[keyNumber]) {

		return true;
	}

	return false;
}

bool Input::ReleaseKey(BYTE keyNumber) {

	if (!key[keyNumber] && preKey[keyNumber]) {

		return true;
	}

	return false;
}

bool Input::PushButton(PadButton button) {
	if (!gamePadDevice_)
		return false;
	int index = static_cast<int>(button);

	if (index <= static_cast<int>(PadButton::kButtonRightThumb)) {
		// 通常ボタン
		return (padState_.rgbButtons[index] & 0x80) != 0;
	} else {
		// POV（十字キー）
		DWORD pov = padState_.rgdwPOV[0];
		if (pov == 0 && button == PadButton::kButtonUp)
			return true;
		if (pov == 9000 && button == PadButton::kButtonRight)
			return true;
		if (pov == 18000 && button == PadButton::kButtonDown)
			return true;
		if (pov == 27000 && button == PadButton::kButtonLeft)
			return true;
	}
	return false;
}

bool Input::TriggerButton(PadButton button) {
	if (!gamePadDevice_)
		return false;
	int index = static_cast<int>(button);

	if (index <= static_cast<int>(PadButton::kButtonRightThumb)) {
		// 通常ボタン
		bool now = (padState_.rgbButtons[index] & 0x80) != 0;
		bool prev = (prePadState_.rgbButtons[index] & 0x80) != 0;
		return (now && !prev);
	} else {
		// POV（十字キー）
		DWORD povNow = padState_.rgdwPOV[0];
		DWORD povPrev = prePadState_.rgdwPOV[0];

		if (button == PadButton::kButtonUp)
			return (povNow == 0 && povPrev != 0);
		if (button == PadButton::kButtonRight)
			return (povNow == 9000 && povPrev != 9000);
		if (button == PadButton::kButtonDown)
			return (povNow == 18000 && povPrev != 18000);
		if (button == PadButton::kButtonLeft)
			return (povNow == 27000 && povPrev != 27000);
	}
	return false;
}
bool Input::ReleaseButton(PadButton button) {
	if (!gamePadDevice_)
		return false;
	int index = static_cast<int>(button);
	if (index <= static_cast<int>(PadButton::kButtonRightThumb)) {
		// 通常ボタン
		bool now = (padState_.rgbButtons[index] & 0x80) != 0;
		bool prev = (prePadState_.rgbButtons[index] & 0x80) != 0;
		return (!now && prev);
	} else {
		// POV（十字キー）
		DWORD povNow = padState_.rgdwPOV[0];
		DWORD povPrev = prePadState_.rgdwPOV[0];
		if (button == PadButton::kButtonUp)
			return (povNow != 0 && povPrev == 0);
		if (button == PadButton::kButtonRight)
			return (povNow != 9000 && povPrev == 9000);
		if (button == PadButton::kButtonDown)
			return (povNow != 18000 && povPrev == 18000);
		if (button == PadButton::kButtonLeft)
			return (povNow != 27000 && povPrev == 27000);
	}
	return false;
}
float Input::GetJoyStickLX() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lX - 32767.0f) / 32767.0f;
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return norm;
}

float Input::GetJoyStickLY() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lY - 32767.0f) / 32767.0f;
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	// DirectInputは上が小さい値なので上下逆にしたいならマイナスをかける
	return -norm;
}

Vector2 Input::GetJoyStickLXY() const { return Vector2(GetJoyStickLX(), GetJoyStickLY()); }

float Input::GetJoyStickRX() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lRx - 32767.0f) / 32767.0f; // 右スティックX
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return norm;
}

float Input::GetJoyStickRY() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = (padState_.lRy - 32767.0f) / 32767.0f; // 右スティックY
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return -norm; // 上をプラスにしたいなら符号を反転
}

Vector2 Input::GetJoyStickRXY() const { return Vector2(GetJoyStickRX(), GetJoyStickRY()); }
float Input::GetLeftTrigger() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = static_cast<float>(padState_.lZ) / 65535.0f;
	if (norm < 0.0f)
		norm = 0.0f;
	if (norm > 1.0f)
		norm = 1.0f;
	return norm;
}

float Input::GetRightTrigger() const {
	if (!gamePadDevice_)
		return 0.0f;
	float norm = static_cast<float>(padState_.lRz) / 65535.0f;
	if (norm < 0.0f)
		norm = 0.0f;
	if (norm > 1.0f)
		norm = 1.0f;
	return norm;
}
bool Input::PushLeftTrigger(float threshold) const { return GetLeftTrigger() >= threshold; }

bool Input::PushRightTrigger(float threshold) const { return GetRightTrigger() >= threshold; }

bool Input::TriggerLeftTrigger(float threshold) const { return GetLeftTrigger() >= threshold && static_cast<float>(prePadState_.lZ) / 65535.0f < threshold; }

bool Input::TriggerRightTrigger(float threshold) const { return GetRightTrigger() >= threshold && static_cast<float>(prePadState_.lRz) / 65535.0f < threshold; }
void Input::SetDeadZone(float deadZone) {
	if (deadZone < 0.0f)
		deadZone = 0.0f;
	if (deadZone > 1.0f)
		deadZone = 1.0f;

	deadZone_ = deadZone;
}

float Input::GetMouseX() const { return static_cast<float>(mouseX_); }

float Input::GetMouseY() const { return static_cast<float>(mouseY_); }

Vector2 Input::GetMouseMove() const {
	// DIMOUSESTATE2 の lX, lY は相対移動量
	return Vector2(static_cast<float>(mouseState_.lX), static_cast<float>(mouseState_.lY));
}
float Input::GetMouseWheelDelta() const { return static_cast<float>(mouseState_.lZ); }

bool Input::PushMouseButton(MouseButton button) const {
	int index = static_cast<int>(button);
	if (index < 0 || index >= static_cast<int>(MouseButton::kMaxButtons)) {
		return false;
	}
	return (mouseState_.rgbButtons[index] & 0x80) != 0;
}

bool Input::TriggerMouseButton(MouseButton button) const {
	int index = static_cast<int>(button);
	if (index < 0 || index >= static_cast<int>(MouseButton::kMaxButtons)) {
		return false;
	}
	bool now = (mouseState_.rgbButtons[index] & 0x80) != 0;
	bool prev = (prevMouseState_.rgbButtons[index] & 0x80) != 0;
	return (now && !prev);
}
bool Input::ReleaseMouseButton(MouseButton button) const {
	int index = static_cast<int>(button);
	if (index < 0 || index >= static_cast<int>(MouseButton::kMaxButtons)) {
		return false;
	}
	bool now = (mouseState_.rgbButtons[index] & 0x80) != 0;
	bool prev = (prevMouseState_.rgbButtons[index] & 0x80) != 0;
	return (!now && prev);
}