#include "DirectInput.h"
#include <assert.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void DirectInput::Initialize(WNDCLASS wc, HWND hwnd) {

	IDirectInput8* directInput = nullptr;
	result_ = DirectInput8Create(wc_.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result_));

	keyboard = nullptr;
	result_ = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result_));
	result_ = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result_));
	result_ = keyboard->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result_));
}
void DirectInput::Update() { 

	keyboard->Acquire();
	BYTE key[256];
	keyboard->GetDeviceState(sizeof(key), key);

	if (key[DIK_0]) {
		OutputDebugStringA("Hit 0\n");
	}


}
bool KeyDown(uint8_t KeyNumber) { return key[KeyNumber] & 0x80; }

bool KeyUp(uint8_t KeyNumber) { return !(key[KeyNumber] & 0x80); }

bool KeyTriggerDown(uint8_t KeyNumber) { return (key[KeyNumber] & 0x80) && !(preKey[KeyNumber] & 0x80); }

bool KeyTriggerUp(uint8_t KeyNumber) { return !(key[KeyNumber] & 0x80) && (preKey[KeyNumber] & 0x80); }
