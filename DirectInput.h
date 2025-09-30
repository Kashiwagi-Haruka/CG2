#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <cstdint>
class DirectInput {

	HRESULT result_;

	IDirectInputDevice8* keyboard;

	


	public:

	

	void Initialize(WNDCLASS wc, HWND hwnd);
	    void Update(uint8_t* key, uint8_t* preKey);
	

	
};


bool KeyDown(uint8_t KeyNumber, const uint8_t* key);
bool KeyUp(uint8_t KeyNumber, const uint8_t* key);
bool KeyTriggerDown(uint8_t KeyNumber, const uint8_t* key, const uint8_t* preKey);
bool KeyTriggerUp(uint8_t KeyNumber, const uint8_t* key, const uint8_t* preKey);

