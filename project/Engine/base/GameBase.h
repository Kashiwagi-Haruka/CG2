#pragma once
#include "Function.h"
#include "Vector4.h"
#include "VertexData.h"
#include "Audio.h"
#include "Input.h"
#include "BlendModeManeger.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Camera.h"
#include <memory>
class SrvManager;
class SpriteCommon;
class Object3dCommon;
class ModelCommon;
class ImGuiManager;

class GameBase{

private:

	static std::unique_ptr<GameBase> instance;

	std::unique_ptr<WinApp> winApp_ = nullptr;

	std::unique_ptr<DirectXCommon> dxCommon_ = nullptr;
	std::unique_ptr<SrvManager> srvManager_ = nullptr;

	
	
	std::unique_ptr<SpriteCommon> spriteCommon_ = nullptr;
	std::unique_ptr<Object3dCommon> obj3dCommon_ = nullptr;
	
	std::unique_ptr<ImGuiManager> imguiM_ = nullptr;
	
	HRESULT hr_;
	

		// バッファの設定
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_RESOURCE_DESC resourceDesc;

	std::unique_ptr<Input> DInput;

   public:	

	static GameBase* GetInstance();

	void Finalize();
	
	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	void BeginFlame(); // フレームの開始処理（commandListリセットなど）
	void EndFlame();   // フレームの終了処理（Present、フェンス待ちなど）

	bool ProcessMessage();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void SpriteCommonSet();
	void ModelCommonSet();
	
	void SetDefaultCamera(Camera* camera);

	

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
	bool ReleaseKey(BYTE keyNumber);
	bool PushButton(Input::PadButton buttonNumber);
	bool TriggerButton(Input::PadButton buttonNumber);

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


	/// <summary>
	/// デッドゾーンの設定
	/// </summary>
	/// <param name="deadZone">初期値は0.2f</param>
	void SetDeadZone(float deadZone);

	// マウス入力関連

	bool PushMouseButton(Input::MouseButton button) const;
	bool TriggerMouseButton(Input::MouseButton button) const;
	float GetMouseX() const;
	float GetMouseY() const;
	Vector2 GetMouseMove() const;

	SpriteCommon* GetSpriteCommon() { return spriteCommon_.get(); };
	Object3dCommon* GetObject3dCommon() { return obj3dCommon_.get(); };
	

private:
	
	
	void SetupPSO();

};

