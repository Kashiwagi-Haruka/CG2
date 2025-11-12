#include "WinApp.h"
#include <d3d12.h>
#include <strsafe.h>
#include <dbt.h> 

#pragma comment(lib, "winmm.lib")

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lparam);

LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	/*if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}*/

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	
	case WM_DEVICECHANGE:
		if (wparam == DBT_DEVICEARRIVAL || wparam == DBT_DEVICEREMOVECOMPLETE || wparam == DBT_DEVNODES_CHANGED) {
			// パッド再列挙フラグを立てる
			WinApp* winApp = reinterpret_cast<WinApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (winApp) {
				winApp->IsPad_ = true;
			}
		}
		break;

	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


void WinApp::Initialize() { 
	HRESULT hr = CoInitializeEx(0, COINITBASE_MULTITHREADED);

	wc_.lpfnWndProc = WindowProc;

	wc_.lpszClassName = L"CG2WindowClass";

	wc_.hInstance = GetModuleHandle(nullptr);

	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc_);

	wrc_ = {0, 0, kClientWidth, kClientHeight};

	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(wc_.lpszClassName, TitleName_, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc_.right - wrc_.left, wrc_.bottom - wrc_.top, nullptr, nullptr, wc_.hInstance, nullptr);
	

	ShowWindow(hwnd_, SW_SHOW);
	SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	timeBeginPeriod(1); // タイマーの精度を1msに設定
}

void WinApp::Update() {
	
	
	
}

void WinApp::Finalize() {
	CloseWindow(hwnd_);
	/*CoUninitialize(); */

}

bool WinApp::ProcessMessage() {
	
	MSG msg{};
	
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		return false;
	}


	return true;
}

