#define NOMINMAX
#include "GameBase.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ModelManeger.h"
#include "TextureManager.h" 

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

GameBase::~GameBase(){
	ResourceRelease();
	delete spriteCommon_;
	delete obj3dCommon_;
	delete DInput;
	ModelManeger::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	delete dxCommon_;
	delete winApp_;
}

void GameBase::Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight) {

	winApp_ = new WinApp();
	winApp_->Initialize();

	dxCommon_ = new DirectXCommon();
	dxCommon_->initialize(winApp_);
	TextureManager::GetInstance()->Initialize(dxCommon_);
	
	
	DInput = new Input();
	DInput->Initialize(winApp_);

	audio.InitializeIXAudio();

	ModelManeger::GetInstance()->Initialize(dxCommon_);
	obj3dCommon_ = new Object3dCommon();
	obj3dCommon_->Initialize(dxCommon_);
	spriteCommon_ = new SpriteCommon();
	spriteCommon_->Initialize(dxCommon_);
	
}

bool GameBase::ProcessMessage() {

	return winApp_->ProcessMessage();
}


LONG WINAPI GameBase::ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻に名前を入れたファイルを作成。Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t filePath[MAX_PATH] = {0};
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION mindumpInformation{0};
	mindumpInformation.ThreadId = threadId;
	mindumpInformation.ExceptionPointers = exception;
	mindumpInformation.ClientPointers = TRUE;
	// DUMPを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &mindumpInformation, nullptr, nullptr);
	// 他に関連づけられてるSEH例外ハンドラがあれば実行。通常はプロセスを終了する。

	return EXCEPTION_EXECUTE_HANDLER;
}

void GameBase::CheackResourceLeaks() {

	Microsoft::WRL::ComPtr <IDXGIDebug1> debug;
	
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {

		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}


}
void GameBase::ResourceRelease() {
	
	TextureManager::GetInstance()->Finalize();
	dxCommon_->Finalize();

	winApp_->Finalize();
}


void GameBase::SetDirectionalLightData(const DirectionalLight& directionalLight) { dxCommon_->SetDirectionalLightData(directionalLight); }

void GameBase::SpriteCommonSet() { spriteCommon_->DrawCommon(); }
void GameBase::ModelCommonSet() { obj3dCommon_->DrawCommon(); }

// 球体用リソース
void GameBase::BeginFlame() { 
	dxCommon_->PreDraw();
	DInput->Update();
}

// --- フレーム終了: ImGui 描画 → Present → フェンス同期まで ---
void GameBase::EndFlame() { dxCommon_->PostDraw(); }

SoundData GameBase::SoundLoadWave(const char* filename){

	return audio.SoundLoadWave(filename);

}
void GameBase::SoundUnload(SoundData* soundData){

	audio.SoundUnload(soundData);

}
void GameBase::SoundPlayWave(const SoundData& sounddata) {
	assert(audio.GetIXAudio2() != nullptr); // 安全のため追加
	audio.SoundPlayWave(audio.GetIXAudio2().Get(), sounddata);
}

bool GameBase::PushMouseButton(Input::MouseButton button) const { return DInput->PushMouseButton(button); }

bool GameBase::TriggerMouseButton(Input::MouseButton button) const { return DInput->TriggerMouseButton(button); }

float GameBase::GetMouseX() const { return DInput->GetMouseX(); };
float GameBase::GetMouseY() const { return DInput->GetMouseY(); };
Vector2 GameBase::GetMouseMove() const { return DInput->GetMouseMove(); };

void GameBase::SetBlendMode(BlendMode mode) { dxCommon_->SetBlendMode(mode); }

bool GameBase::PushKey(BYTE keyNumber){ return DInput->PushKey(keyNumber); }
bool GameBase::TriggerKey(BYTE keyNumber) { return DInput->TriggerKey(keyNumber); }
bool GameBase::PushButton(Input::PadButton button) { return DInput->PushButton(button); }
bool GameBase::TriggerButton(Input::PadButton button) { return DInput->TriggerButton(button); }
// ジョイスティック

float GameBase::GetJoyStickLX() const { return DInput->GetJoyStickLX(); };


float GameBase::GetJoyStickLY() const { return DInput->GetJoyStickLY(); };

Vector2 GameBase::GetJoyStickLXY() const { return DInput->GetJoyStickLXY(); };

float GameBase::GetJoyStickRX() const { return DInput->GetJoyStickRX(); };
float GameBase::GetJoyStickRY() const { return DInput->GetJoyStickRY(); };
Vector2 GameBase::GetJoyStickRXY() const { return DInput->GetJoyStickRXY(); };


/// <summary>
/// デッドゾーンの設定
/// </summary>
/// <param name="deadZone">初期値は0.2f</param>
void GameBase::SetDeadZone(float deadZone) { DInput->SetDeadZone(deadZone); };

void GameBase::DrawMesh(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world){ 

	dxCommon_->DrawMesh(vertices, color, textureHandle, wvp, world);
}
void GameBase::DrawParticle(const std::vector<VertexData>& vertices, uint32_t color, uint32_t textureHandle, const Matrix4x4& wvp, const Matrix4x4& world, int instanceCount) {
	dxCommon_->DrawParticle(vertices, color, textureHandle, wvp, world, instanceCount);
}
void GameBase::DrawSphere(const Vector3& center, float radius, uint32_t color, int textureHandle, const Matrix4x4& viewProj) {
	dxCommon_->DrawSphere(center, radius, color, textureHandle, viewProj);
}
