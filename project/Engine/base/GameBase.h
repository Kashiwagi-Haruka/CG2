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

class SpriteCommon;
class Object3dCommon;

class GameBase{

private:

	WinApp* winApp_ = nullptr;

	DirectXCommon* dxCommon_ = nullptr;
	
	Audio audio;
	
	SpriteCommon* spriteCommon_ = nullptr;
	Object3dCommon* modelCommon_ = nullptr;

	
	HRESULT hr_;
	

		// バッファの設定
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_RESOURCE_DESC resourceDesc;

	Input* DInput;

   public:	

	   ~GameBase();
	
	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	void BeginFlame(); // フレームの開始処理（commandListリセットなど）
	void EndFlame();   // フレームの終了処理（Present、フェンス待ちなど）

	bool ProcessMessage();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void CheackResourceLeaks();

	void ResourceRelease();

	// 描画
	void DrawTriangle(const Vector3 positions[3], const Vector2 texcoords[3], const Vector4& color, int textureHandle);
	
	

	SoundData SoundLoadWave(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayWave(const SoundData& sounddata);


	void SpriteCommonSet();
	void ModelCommonSet();
	

	Transform GetCameraTransform() const { return dxCommon_->GetCameraTransform(); };

	

	void DrawSphere(const Vector3& center, float radius, uint32_t color, int textureHandle, const Matrix4x4& viewProj);
	void DrawSphere(const Vector3& center, const Vector3& radius, const Vector3& rotation, uint32_t color, int textureHandle, const Matrix4x4& viewProj);
	void DrawMesh(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world);
	void DrawParticle(const std::vector<VertexData>& vertices, uint32_t color, uint32_t textureHandle, const Matrix4x4& wvp, const Matrix4x4& world, int instanceCount);
	
	
	void SetDirectionalLightData(const DirectionalLight& directionalLight);


	

	void SetBlendMode(BlendMode blendMode);

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
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

	SpriteCommon* GetSpriteCommon() { return spriteCommon_; };
	Object3dCommon* GetModelCommon() { return modelCommon_; };

private:


	
	void CreateModelResources();
	
	
	void SetupPSO();

	
	
	/*void CreateSpriteVertexBuffer();*/
	
	

};

