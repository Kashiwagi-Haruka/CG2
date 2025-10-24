#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <Windows.h>
#include <cstdint>
#include <dinput.h>
#include <wrl.h>
#include "Vector2.h"
#include "WinApp.h"

class Input {

	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<IDirectInput8> directInput = nullptr;
	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInputDevice8> gamePadDevice_;
	ComPtr<IDirectInputDevice8> mouseDevice_;

	BYTE key[256] = {0};
	BYTE preKey[256] = {0};

	DIJOYSTATE padState_{};    // ゲームパッドの現在の状態
	DIJOYSTATE prePadState_{}; // 前フレームの状態

	float deadZone_ = 0.2f; // デッドゾーン(0.0f～1.0f)
	
	DIMOUSESTATE2 mouseState_{};
	DIMOUSESTATE2 prevMouseState_{};
	LONG mouseX_ = 0;
	LONG mouseY_ = 0;
	int noPadCounter = 0;

	WinApp* winApp_ = nullptr;

public:

	enum class PadButton {
		kButtonA = 0,         // rgbButtons[0]
		kButtonB,             // rgbButtons[1]
		kButtonX,             // rgbButtons[2]
		kButtonY,             // rgbButtons[3]
		kButtonLeftShoulder,  // rgbButtons[4]
		kButtonRightShoulder, // rgbButtons[5]
		kButtonBack,          // rgbButtons[6]
		kButtonStart,         // rgbButtons[7]
		kButtonLeftThumb,     // rgbButtons[8]
		kButtonRightThumb,    // rgbButtons[9]

		// 十字キーは POV（rgdwPOV[0]）を別判定で扱う
		kButtonUp,
		kButtonDown,
		kButtonLeft,
		kButtonRight,

		kMaxButtons
	};

	enum class MouseButton {
		kLeft = 0,
		kRight,
		kMiddle,
		kMaxButtons };

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="wc"></param>
	/// <param name="hwnd"></param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// キーボード
	
	/// <summary>
	/// キーが押されているか
	/// </summary>
	/// <param name="keyNumber">キーコード</param>
	/// <returns></returns>
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// キーが押された瞬間か
	/// </summary>
	/// <param name="keyNumber"></param>
	/// <returns></returns>
	bool TriggerKey(BYTE keyNumber);

	// ゲームパッド
	bool PushButton(PadButton button);                    // ボタンが押されているか
	bool TriggerButton(PadButton button);                 // ボタンが押された瞬間か

	//ジョイスティック

	/// <summary>
	/// ジョイスティックのX軸を取得
	/// </summary>
	/// <returns></returns>
	float GetJoyStickLX() const;

	/// <summary>
	/// ジョイスティックのY軸を取得
	/// </summary>
	/// <returns></returns>
	float GetJoyStickLY() const;
	/// <summary>
	/// Vector2でジョイスティックのXYを取得
	/// </summary>
	/// <returns></returns>
	Vector2 GetJoyStickLXY() const;


	// 右スティック
	float GetJoyStickRX() const;
	float GetJoyStickRY() const;
	Vector2 GetJoyStickRXY() const;

	// マウス
	float GetMouseX() const; // マウスのX座標を取得
	float GetMouseY() const; // マウスのY座標を取得
	Vector2 GetMouseMove()const; // マウスの移動量を取得
	bool PushMouseButton(MouseButton button) const; // マウスボタンが押されているか
	bool TriggerMouseButton(MouseButton button) const; // マウスボタンが押された瞬間か

	/// <summary>
	/// デッドゾーンの設定
	/// </summary>
	/// <param name="deadZone">初期値は0.2f</param>
	void SetDeadZone(float deadZone);

private:
	// デバイス列挙用の static コールバック
	static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
};
