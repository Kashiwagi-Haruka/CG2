#define NOMINMAX
#include "GameBase.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "ParticleManager.h"
#include "SpriteCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <DbgHelp.h>
#include <dxgidebug.h>
#include <strsafe.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

std::unique_ptr<GameBase> GameBase::instance = nullptr;

GameBase* GameBase::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<GameBase>();
	}
	return instance.get();
}
void GameBase::Finalize() {

	imguiM_->Finalize();
	Audio::GetInstance()->Finalize();

	TextureManager::GetInstance()->Finalize();

	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	SpriteCommon::GetInstance()->Finalize();
	Object3dCommon::GetInstance()->Finalize();

	dxCommon_->Finalize();
	winApp_->Finalize();
	instance.reset();
}

void GameBase::Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight) {

	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize(TitleName);

	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->initialize(winApp_.get());
	srvManager_ = std::make_unique<SrvManager>();
	srvManager_->Initialize(dxCommon_.get());

	Input::GetInstance()->Initialize(winApp_.get());
	imguiM_ = std::make_unique<ImGuiManager>();
	imguiM_->Initialize(winApp_.get(), dxCommon_.get(), srvManager_.get());
	Audio::GetInstance()->InitializeIXAudio();
	TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());
	ParticleManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());
	ModelManager::GetInstance()->Initialize(dxCommon_.get());

	Object3dCommon::GetInstance()->Initialize(dxCommon_.get());
	SpriteCommon::GetInstance()->Initialize(dxCommon_.get());
}

bool GameBase::ProcessMessage() { return winApp_->ProcessMessage(); }

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

void GameBase::BeginFlame() {

	dxCommon_->PreDraw();
	imguiM_->Begin();
	Input::GetInstance()->Update();
	Audio::GetInstance()->Update();
}

// --- フレーム終了: ImGui 描画 → Present → フェンス同期まで ---
void GameBase::EndFlame() {
	imguiM_->End();
	imguiM_->Draw(srvManager_.get(), dxCommon_.get());
	dxCommon_->PostDraw();
}

float GameBase::GetDeltaTime() { return dxCommon_->GetDeltaTime(); }