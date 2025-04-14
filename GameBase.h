#pragma once
#include <Windows.h>
#include <cstdint>
#include <format>
#include "ConvertString.h"


class GameBase {

private:
	WNDCLASS wc{};
	RECT wrc;
	HWND hwnd;
	MSG msg{};
	ConvertString* CStr{};
	std::wstring wstringValue = L"k";



public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void Log(const std::string& message);

	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	bool IsMsgQuit();

	void OutPutLog();

	

	MSG* GetMsg() { return &msg; };
};

