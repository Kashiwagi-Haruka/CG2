#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <cstdint>
class DirectInput {

	HRESULT result_;
	WNDCLASS wc_;
	HWND hwnd_;
	IDirectInputDevice8* keyboard;

	


	public:

	

	void Initialize(WNDCLASS wc, HWND hwnd);
	void Update();
	

	
};

BYTE key[256]{};
BYTE preKey[256]{};
bool KeyDown(uint8_t KeyNumber);
bool KeyUp(uint8_t KeyNumber);
bool KeyTriggerDown(uint8_t KeyNumber);
bool KeyTriggerUp(uint8_t KeyNumber);