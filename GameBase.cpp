#include "GameBase.h"

LRESULT CALLBACK GameBase::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	switch (msg) {

	case WM_DESTROY:

		PostQuitMessage(0);

		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void GameBase::Log(const std::string& message) {	

	OutputDebugStringA(message.c_str()); 
}

void GameBase::Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight) {

	wc.lpfnWndProc = WindowProc;

	wc.lpszClassName = L"CG2WindowClass";

	wc.hInstance = GetModuleHandle(nullptr);

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);


	wrc = {0, 0, WindowWidth, WindowHeight};

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(wc.lpszClassName, TitleName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);

	ShowWindow(hwnd, SW_SHOW);
}

bool GameBase::IsMsgQuit() {

	if (msg.message != WM_QUIT) {
	
		return true;
	}

	return false;
}

void GameBase::OutPutLog() {


	// 出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");
	Log(CStr->ConvertString_(std::format(L"WSTRING {}\n", wstringValue)));
}
