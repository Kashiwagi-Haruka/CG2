#include "Input.h"
#include <cassert>

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

	
	prePadState_ = padState_;

	if (gamePadDevice_) {
		gamePadDevice_->Acquire();
		gamePadDevice_->GetDeviceState(sizeof(DIJOYSTATE),&padState_);
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
	int idx = static_cast<int>(button);
	if (idx < 0 || idx >= static_cast<int>(PadButton::kMaxButtons))
		return false;
	return (padState_.rgbButtons[idx] & 0x80) != 0;
}

bool Input::TriggerButton(PadButton button) {
	int idx = static_cast<int>(button);
	if (idx < 0 || idx >= static_cast<int>(PadButton::kMaxButtons))
		return false;
	return (padState_.rgbButtons[idx] & 0x80) && !(prePadState_.rgbButtons[idx] & 0x80);
}
