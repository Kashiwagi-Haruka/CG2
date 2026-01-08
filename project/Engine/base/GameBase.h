#pragma once
#include "Audio.h"
#include "BlendMode/BlendModeManager.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Input.h"
#include "TextureManager.h"
#include "Vector4.h"
#include "VertexData.h"
#include "WinApp.h"
#include <memory>
class SrvManager;
class SpriteCommon;
class ModelCommon;
class ImGuiManager;

class GameBase {

private:
	static std::unique_ptr<GameBase> instance;

	std::unique_ptr<WinApp> winApp_ = nullptr;

	std::unique_ptr<DirectXCommon> dxCommon_ = nullptr;
	std::unique_ptr<SrvManager> srvManager_ = nullptr;

	std::unique_ptr<ImGuiManager> imguiM_ = nullptr;

	HRESULT hr_;

	std::unique_ptr<Input> DInput;

public:
	static GameBase* GetInstance();

	void Finalize();

	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	void BeginFlame(); // フレームの開始処理(commandListリセットなど)
	void EndFlame();   // フレームの終了処理(Present、フェンス待ちなど)

	bool ProcessMessage();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	
	
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
	bool ReleaseKey(BYTE keyNumber);
	bool PushButton(Input::PadButton buttonNumber);
	bool TriggerButton(Input::PadButton buttonNumber);
	bool ReleaseButton(Input::PadButton buttonNumber);

	// ジョイスティック

	/// <summary>
	/// 左ジョイスティックのX軸を取得
	/// </summary>
	/// <returns></returns>
	float GetJoyStickLX() const;

	/// <summary>
	/// 左ジョイスティックのY軸を取得
	/// </summary>
	/// <returns></returns>
	float GetJoyStickLY() const;
	/// <summary>
	/// Vector2で左ジョイスティックのXYを取得
	/// </summary>
	/// <returns></returns>
	Vector2 GetJoyStickLXY() const;

	// 右スティック

	/// <summary>
	/// 右ジョイスティックのX軸を取得
	/// </summary>
	/// <returns></returns>
	float GetJoyStickRX() const;

	/// <summary>
	/// 右ジョイスティックのY軸を取得
	/// </summary>
	/// <returns></returns>
	float GetJoyStickRY() const;

	/// <summary>
	/// Vector2で右ジョイスティックのXYを取得
	/// </summary>
	/// <returns></returns>
	Vector2 GetJoyStickRXY() const;

	// トリガー (RT/LT)

	/// <summary>
	/// 右トリガー(RT)の値を取得 (0.0f～1.0f)
	/// </summary>
	float GetRightTrigger() const;

	/// <summary>
	/// 左トリガー(LT)の値を取得 (0.0f～1.0f)
	/// </summary>
	float GetLeftTrigger() const;

	/// <summary>
	/// 右トリガー(RT)が押されているか
	/// </summary>
	bool PushRightTrigger() const;

	/// <summary>
	/// 左トリガー(LT)が押されているか
	/// </summary>
	bool PushLeftTrigger() const;

	/// <summary>
	/// 右トリガー(RT)が押された瞬間か
	/// </summary>
	bool TriggerRightTrigger() const;

	/// <summary>
	/// 左トリガー(LT)が押された瞬間か
	/// </summary>
	bool TriggerLeftTrigger() const;

	/// <summary>
	/// デッドゾーンの設定
	/// </summary>
	/// <param name="deadZone">初期値は0.2f</param>
	void SetDeadZone(float deadZone);

	/// <summary>
	/// トリガーの閾値を設定
	/// </summary>
	/// <param name="threshold">初期値は0.3f (0.0f～1.0f)</param>
	void SetTriggerThreshold(float threshold);

	// マウス入力関連

	bool PushMouseButton(Input::MouseButton button) const;
	bool TriggerMouseButton(Input::MouseButton button) const;
	bool ReleaseMouseButton(Input::MouseButton button) const;
	void SetIsCursorStablity(bool isCursor);
	void SetIsCursorVisible(bool isVisible);
	float GetMouseX() const;
	float GetMouseY() const;
	Vector2 GetMouseMove() const;

	
	
};