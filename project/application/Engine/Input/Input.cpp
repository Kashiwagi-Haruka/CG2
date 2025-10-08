#include "Input.h"
#include <cassert>
#include <cmath>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;


BOOL CALLBACK Input::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
	Input* input = reinterpret_cast<Input*>(pContext);
	HRESULT hr = input->directInput->CreateDevice(pdidInstance->guidInstance, &input->gamePadDevice_, NULL);
	if (FAILED(hr)) {
		return DIENUM_CONTINUE;
	}
	return DIENUM_STOP;
}

void Input::Initialize(WNDCLASS wc, HWND hwnd) {
	HRESULT result;

	// DirectInput 作成
	result = DirectInput8Create(wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボード
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// ゲームパッド列挙 → 最初の1台を使用
	directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);

	if (gamePadDevice_) {
		result = gamePadDevice_->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(result));
		result = gamePadDevice_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		assert(SUCCEEDED(result));
	}
}


void Input::Update() {
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
				// デバイスが切断されたか、状態が取れない
				gamePadDevice_.Reset();
			}
		} else {
			// Acquire 自体失敗 → デバイスなしとみなす
			gamePadDevice_.Reset();
		}
	}

	// --- もしデバイスが切れていたら再列挙 ---
	if (!gamePadDevice_) {
		directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
		if (gamePadDevice_) {
			gamePadDevice_->SetDataFormat(&c_dfDIJoystick);
			gamePadDevice_->SetCooperativeLevel(GetActiveWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
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

bool Input::PushButton(PadButton button) {
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

float Input::GetJoyStickLX() const {
	// 中央 32767 を基準に正規化
	float norm = (padState_.lX - 32767.0f) / 32767.0f;
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return norm;
}

float Input::GetJoyStickLY() const {
	float norm = (padState_.lY - 32767.0f) / 32767.0f;
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	// DirectInputは上が小さい値なので上下逆にしたいならマイナスをかける
	return -norm;
}

Vector2 Input::GetJoyStickLXY() const {
	return Vector2(GetJoyStickLX(), GetJoyStickLY()); 
}

float Input::GetJoyStickRX() const {
	float norm = (padState_.lRx - 32767.0f) / 32767.0f; // 右スティックX
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return norm;
}

float Input::GetJoyStickRY() const {
	float norm = (padState_.lRy - 32767.0f) / 32767.0f; // 右スティックY
	if (fabs(norm) < deadZone_)
		norm = 0.0f;
	return -norm; // 上をプラスにしたいなら符号を反転
}

Vector2 Input::GetJoyStickRXY() const { return Vector2(GetJoyStickRX(), GetJoyStickRY()); }


void Input::SetDeadZone(float deadZone){
	if (deadZone < 0.0f)
		deadZone = 0.0f;
	if (deadZone > 1.0f)
		deadZone = 1.0f;
	
	deadZone_ = deadZone;
}