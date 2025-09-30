#include "DirectInput.h"

#define DIRECTINPUT_VERSION 0x0800
#include <cassert>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")


IDirectInput8* directInput = nullptr;

void DirectInput::Initialize(WNDCLASS wc, HWND hwnd) {
	// DirectInputオブジェクトの生成
	HRESULT result = DirectInput8Create(wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void DirectInput::Update(uint8_t* key, uint8_t* preKey) {
	// 前のフレームのキー入力を保存
	memcpy(preKey, key, 256);

	// キーボードの状態を取得
	keyboard->Acquire();
	BYTE temp[256];
	keyboard->GetDeviceState(sizeof(temp), temp);

	// 外から渡された key 配列にコピー
	memcpy(key, temp, 256);
}

// 補助関数（外部から渡された key を使う場合）
bool KeyDown(uint8_t KeyNumber, const uint8_t* key) { return key[KeyNumber] & 0x80; }

bool KeyUp(uint8_t KeyNumber, const uint8_t* key) { return !(key[KeyNumber] & 0x80); }

bool KeyTriggerDown(uint8_t KeyNumber, const uint8_t* key, const uint8_t* preKey) { return (key[KeyNumber] & 0x80) && !(preKey[KeyNumber] & 0x80); }

bool KeyTriggerUp(uint8_t KeyNumber, const uint8_t* key, const uint8_t* preKey) { return !(key[KeyNumber] & 0x80) && (preKey[KeyNumber] & 0x80); }
