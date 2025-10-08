#pragma once
#include <Windows.h>
#include "imGuiM.h"
#include <cstdint>
#include <wrl.h>
class WinApp {

private:

	WNDCLASS wc_{};
	RECT wrc_;
	HWND hwnd_;
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
	const wchar_t* TitleName_ = L"CG2";

	bool IsPad_ = false;

public:
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public: 

	//初期化
	void Initialize();
	// 更新
	void Update();
	// 終了
	void Finalize();

	// メッセージの処理
	bool ProcessMessage();

	//getter
	HWND GetHwnd() const{ return hwnd_; };
	//getter
	HINSTANCE GetHinstance() const { return wc_.hInstance; };
	
	bool GetIsPad() { return IsPad_; };
	void SetIsPad(bool isPad) { IsPad_ = isPad; };

private:

	void DebugLayer();


	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


};
