#define NOMINMAX
#include "WinApp.h"
#include <algorithm>
#include <d3d12.h>
#include <dbt.h>
#include <strsafe.h>
#ifdef USE_IMGUI
#include "externals/imgui/imgui_impl_win32.h"
#endif // USE_IMGUI

#pragma comment(lib, "winmm.lib")
#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lparam);
#endif // USE_IMGUI

namespace {
constexpr float kTargetAspectRatio = 16.0f / 9.0f;
bool gIsFullscreen = false;
RECT gWindowedRect{};
DWORD gWindowedStyle = 0;

void ToggleFullscreen(HWND hwnd) {
	if (!gIsFullscreen) {
		gWindowedStyle = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_STYLE));
		GetWindowRect(hwnd, &gWindowedRect);

		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitorInfo);

		SetWindowLongPtr(hwnd, GWL_STYLE, static_cast<LONG_PTR>(gWindowedStyle & ~WS_OVERLAPPEDWINDOW));
		SetWindowPos(
		    hwnd, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
		    SWP_FRAMECHANGED);
		gIsFullscreen = true;
	} else {
		SetWindowLongPtr(hwnd, GWL_STYLE, static_cast<LONG_PTR>(gWindowedStyle));
		SetWindowPos(
		    hwnd, nullptr, gWindowedRect.left, gWindowedRect.top, gWindowedRect.right - gWindowedRect.left, gWindowedRect.bottom - gWindowedRect.top,
		    SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER);
		gIsFullscreen = false;
	}
}
void KeepClientAspectRatio16By9(HWND hwnd, WPARAM edge, RECT* rect) {
	if (!rect) {
		return;
	}

	RECT currentWindowRect{};
	RECT currentClientRect{};
	if (!GetWindowRect(hwnd, &currentWindowRect) || !GetClientRect(hwnd, &currentClientRect)) {
		return;
	}

	const int32_t nonClientWidth = (currentWindowRect.right - currentWindowRect.left) - (currentClientRect.right - currentClientRect.left);
	const int32_t nonClientHeight = (currentWindowRect.bottom - currentWindowRect.top) - (currentClientRect.bottom - currentClientRect.top);

	const int32_t windowWidth = rect->right - rect->left;
	const int32_t windowHeight = rect->bottom - rect->top;
	int32_t clientWidth = std::max(1, windowWidth - nonClientWidth);
	int32_t clientHeight = std::max(1, windowHeight - nonClientHeight);

	switch (edge) {
	case WMSZ_LEFT:
	case WMSZ_RIGHT:
		clientWidth = static_cast<int32_t>(static_cast<float>(clientHeight) * kTargetAspectRatio);
		break;
	case WMSZ_TOP:
	case WMSZ_BOTTOM:
		clientHeight = static_cast<int32_t>(static_cast<float>(clientWidth) / kTargetAspectRatio);
		break;
	default:
		clientHeight = static_cast<int32_t>(static_cast<float>(clientWidth) / kTargetAspectRatio);
		break;
	}

	const int32_t adjustedWindowWidth = clientWidth + nonClientWidth;
	const int32_t adjustedWindowHeight = clientHeight + nonClientHeight;

	switch (edge) {
	case WMSZ_LEFT:
	case WMSZ_TOPLEFT:
	case WMSZ_BOTTOMLEFT:
		rect->left = rect->right - adjustedWindowWidth;
		break;
	default:
		rect->right = rect->left + adjustedWindowWidth;
		break;
	}

	switch (edge) {
	case WMSZ_TOP:
	case WMSZ_TOPLEFT:
	case WMSZ_TOPRIGHT:
		rect->top = rect->bottom - adjustedWindowHeight;
		break;
	default:
		rect->bottom = rect->top + adjustedWindowHeight;
		break;
	}
}
} // namespace

LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_KEYDOWN && wparam == VK_F11) {
		ToggleFullscreen(hwnd);
		return 0;
	}
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // USE_IMGUI
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZING:
		KeepClientAspectRatio16By9(hwnd, wparam, reinterpret_cast<RECT*>(lparam));
		return TRUE;
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

void WinApp::Initialize(const wchar_t* TitleName) {
	HRESULT hr = CoInitializeEx(0, COINITBASE_MULTITHREADED);

	wc_.lpfnWndProc = WindowProc;

	wc_.lpszClassName = L"CG2WindowClass";

	wc_.hInstance = GetModuleHandle(nullptr);

	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc_);

	wrc_ = {0, 0, kClientWidth, kClientHeight};

	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(wc_.lpszClassName, TitleName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc_.right - wrc_.left, wrc_.bottom - wrc_.top, nullptr, nullptr, wc_.hInstance, nullptr);

	ShowWindow(hwnd_, SW_SHOW);
	SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	timeBeginPeriod(1); // タイマーの精度を1msに設定
}

void WinApp::Update() {}

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
