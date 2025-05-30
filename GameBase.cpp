#include "GameBase.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

LRESULT CALLBACK GameBase::WindowProc(HWND hwnd_, UINT msg_, WPARAM wparam, LPARAM lparam) {

	if (ImGui_ImplWin32_WndProcHandler(hwnd_, msg_, wparam, lparam)) {
		return true;
	}

	switch (msg_) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd_, msg_, wparam, lparam);
}


void GameBase::Log(const std::string& message) {	

	OutputDebugStringA(message.c_str()); 
}

void GameBase::Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight) {

	wc_.lpfnWndProc = WindowProc;

	wc_.lpszClassName = L"CG2WindowClass";

	wc_.hInstance = GetModuleHandle(nullptr);

	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc_);


	wrc_ = {0, 0, WindowWidth, WindowHeight};

	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(wc_.lpszClassName, TitleName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc_.right - wrc_.left, wrc_.bottom - wrc_.top, nullptr, nullptr, wc_.hInstance, nullptr);
	///Debuglayer
	DebugLayer();

	ShowWindow(hwnd_, SW_SHOW);

	// DXGIファクトリーの生成
	dxgiFactory_ = nullptr;
	// HRESULTはWindows系のエラーコードであり、
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる

	hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr_));

	
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr_ = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr_)); // 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			Log(CStr_.ConvertString_(std::format(L"Use Adapater: {}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr; // ソフトウェアアダプタの場合は見なかった。
	}
	// 適切なアダプタが見つからなかったので起動できない。
	assert(useAdapter_ != nullptr);

	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {

	    D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		// 採用したアダプターでデバイスを生成
		hr_ = D3D12CreateDevice(useAdapter_, featureLevels[i], IID_PPV_ARGS(&device_));

		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr_)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel: {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Log("Complete create D3D12Device!!!\n"); // 初期化完了のログをだす
	DebugError();

	WindowClear();
	
}

bool GameBase::IsMsgQuit() {

	if (msg_.message != WM_QUIT) {
	
		return true;
	}

	return false;
}

void GameBase::OutPutLog() {


	// 出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");
	if (FAILED(hr_)) {
		Log("Failed to create fence_. HRESULT: " + std::to_string(hr_));
	}
	Log(CStr_.ConvertString_(std::format(L"WSTRING {}\n", wstringValue_)));
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

void GameBase::WindowClear() {
	// コマンドキューを生成する
	commandQueue_ = nullptr;
	commandQueueDesc_ = {};
	commandQueueDesc_.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc_.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 主に描画用途
	hr_ = device_->CreateCommandQueue(&commandQueueDesc_, IID_PPV_ARGS(&commandQueue_));

	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	// コマンドアロケータを生成する
	commandAllocator_ = nullptr;
	hr_ = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	// コマンドリストを生成する
	commandList_ = nullptr;
	hr_ = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, nullptr, IID_PPV_ARGS(&commandList_));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	swapChain_ = nullptr;
	swapChainDesc_ = {};

	swapChainDesc_.Width = kClientWidth_;
	swapChainDesc_.Height = kClientHeight_;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr_ = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, hwnd_, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr_));

	// ディスクリプタヒープの生成

	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	
	// SRV用ディスクリプタヒープ作成
	srvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256, true);



	//rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
	//rtvDescriptorHeapDesc.NumDescriptors = 2;                    // ダブルバッファ用に2つ。多くてもかまわない
	//hr_ = device_->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	//// ディスクリプタヒープがつくれなかったので起動できない
	//assert(SUCCEEDED(hr_));
	DXCInitialize();

	hr_ = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	// 上手く取得できなければ起動できない
	assert(SUCCEEDED(hr_));
	hr_ = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr_));
	assert(backBufferIndex_ < 2);
	//
	assert(swapChainResources_[0] != nullptr);
	assert(swapChainResources_[1] != nullptr);
	assert(swapChainResources_[backBufferIndex_] != nullptr);
	//

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;      // 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources_[0], &rtvDesc, rtvHandles_[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device_->CreateRenderTargetView(swapChainResources_[1], &rtvDesc, rtvHandles_[1]);

	depthStencilResource_ = CreateDepthStencilTextureResource(device_, kClientWidth_, kClientHeight_);

	dsvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device_->CreateDepthStencilView(depthStencilResource_, &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// ImGui 初期化はここで！
	imguiM_.MInitialize(hwnd_, device_, swapChainDesc_, rtvDesc, srvDescriptorHeap_);

	if (srvDescriptorHeap_ == nullptr) {
		assert(false);
	}
	texture_.Initialize(
	    device_, srvDescriptorHeap_,
	   "C:/Class/Program/DirectXGame/Resources/uvChecker.png");
	GPUHandle_ = texture_.GetGpuHandle();
	assert(GPUHandle_.ptr != 0); // もし0なら SRV 作成に失敗してる

	DrawCommandList();


	CrtvTransitionBarrier();
	

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr_ = commandList_->Close();

	assert(SUCCEEDED(hr_));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = {commandList_};
	commandQueue_->ExecuteCommandLists(1, commandLists);
	// GPUと05に画面の交換を行うよう通知する
	// GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);

// Fenceを作る
	fenceValue_ = 0;
	hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));

	// fenceEvent_ の多重生成を防ぐ！
	if (!fenceEvent_) {
		fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		assert(fenceEvent_ != nullptr);
	}


	// Fenceの値を更新
	fenceValue_++;
	commandQueue_->Signal(fence_, fenceValue_);

	// Fenceの値が指定Signalに達してるか確認
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE); // ★ここでちゃんとGPUが終わるまで待つ！
	}

}
void GameBase::DebugLayer() {

#ifdef _DEBUG



	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
	//デバッグレイヤー
		debugController_->EnableDebugLayer();
		//更にGPU側でもチェックを行うようにする
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}
	

#endif // DEBUG

}

void GameBase::DebugError() {

#ifdef _DEBUG

	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
	//ヤバいエラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
	//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
	//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
	
	//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
		    // https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-window-11
		    D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};
		D3D12_INFO_QUEUE_FILTER filter{};

		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		//指定したメッセージの表示を抑制させる
		infoQueue->PushStorageFilter(&filter);


	//解放
		infoQueue->Release();

	
	}


#endif // _DEBUG



}

void GameBase::CrtvTransitionBarrier() {
	//画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	//今回はRenderTargetからPresentにする。
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);
}

void GameBase::FenceEvent() {
	hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));

	// 二重生成防止
	if (!fenceEvent_) {
		fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		assert(fenceEvent_ != nullptr);
	}

	assert(fenceEvent_ != nullptr);
}



void GameBase::CheackResourceLeaks() {

	IDXGIDebug1* debug;
	
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {

		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}


}
void GameBase::ResourceRelease() {
	// --- GPUが完全に終わるまで待つ（Unmap前に必須） ---
	if (commandQueue_ && fence_ && fenceEvent_) {
		fence_->Signal(fenceValue_);
		if (fence_->GetCompletedValue() < fenceValue_) {
			fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
			WaitForSingleObject(fenceEvent_, INFINITE);
		}
		++fenceValue_;
	}

	// --- Transform のアンマップ ---
	if (transformResource_) {
		transformResource_->Unmap(0, nullptr);
	}



	// --- テクスチャリソース ---
	texture_.Release();

	// --- 各種リソース ---
	if (vertexResource_) {
		vertexResource_->Release();
		vertexResource_ = nullptr;
	}

	if (materialResource_) {
		materialResource_->Release();
		materialResource_ = nullptr;
	}

	if (transformResource_) {
		transformResource_->Release();
		transformResource_ = nullptr;
	}


	if (graphicsPipelineState_) {
		graphicsPipelineState_->Release();
		graphicsPipelineState_ = nullptr;
	}

	if (signatureBlob_) {
		signatureBlob_->Release();
		signatureBlob_ = nullptr;
	}

	if (errorBlob_) {
		errorBlob_->Release();
		errorBlob_ = nullptr;
	}

	if (includeHandler_) {
		includeHandler_->Release();
		includeHandler_ = nullptr;
	}

	if (dxcCompiler_) {
		dxcCompiler_->Release();
		dxcCompiler_ = nullptr;
	}

	if (dxcUtils_) {
		dxcUtils_->Release();
		dxcUtils_ = nullptr;
	}



	if (dsvDescriptorHeap_) {
		dsvDescriptorHeap_->Release();
		dsvDescriptorHeap_ = nullptr;
	}

	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}

	if (pixelShaderBlob_) {
		pixelShaderBlob_->Release();
		pixelShaderBlob_ = nullptr;
	}

	if (vertexShaderBlob_) {
		vertexShaderBlob_->Release();
		vertexShaderBlob_ = nullptr;
	}

	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}

	if (fence_) {
		fence_->Release();
		fence_ = nullptr;
	}

	if (depthStencilResource_) {
		depthStencilResource_->Release();
		depthStencilResource_ = nullptr;
	}

	if (rtvDescriptorHeap_) {
		rtvDescriptorHeap_->Release();
		rtvDescriptorHeap_ = nullptr;
	}

	for (int i = 0; i < 2; ++i) {
		if (swapChainResources_[i]) {
			swapChainResources_[i]->Release();
			swapChainResources_[i] = nullptr;
		}
	}

	if (swapChain_) {
		swapChain_->Release();
		swapChain_ = nullptr;
	}

	if (commandList_) {
		commandList_->Release();
		commandList_ = nullptr;
	}

	if (commandAllocator_) {
		commandAllocator_->Release();
		commandAllocator_ = nullptr;
	}

	if (commandQueue_) {
		commandQueue_->Release();
		commandQueue_ = nullptr;
	}
	imguiM_.Finalize(); // これを一番最後近くに移動する
	if (srvDescriptorHeap_) {
		srvDescriptorHeap_->Release();
		srvDescriptorHeap_ = nullptr;
	}
	if (device_) {
		device_->Release();
		device_ = nullptr;
	}

	if (useAdapter_) {
		useAdapter_->Release();
		useAdapter_ = nullptr;
	}

	if (dxgiFactory_) {
		dxgiFactory_->Release();
		dxgiFactory_ = nullptr;
	}

#ifdef _DEBUG
	if (debugController_) {
		debugController_->Release();
		debugController_ = nullptr;
	}
#endif

	IDXGIDebug1* debug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	CloseWindow(hwnd_);
}




void GameBase::DXCInitialize() {

	// dxcCompilerを初期化
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	hr_ = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr_));
	hr_ = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr_));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler_ = nullptr;
	hr_ = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr_));

	PSO();
	VertexResource();
}

IDxcBlob* GameBase::CompileShader(/* CompilerするShaderファイルへのパス*/const std::wstring& filePath,
    // Compilerに使用するProfile
    const wchar_t* profile,
    // 初期化で生成したものをつかう
    IDxcUtils* dxcUtils_, IDxcCompiler3* dxcCompiler_, IDxcIncludeHandler* includeHandler_) {
	// ここの中身をこの後書いていく
	// 1. hlslファイルを読む
	// // これからシェーダーをコンパイルする旨をログに出す
	Log(CStr_.ConvertString_(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr_ = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	// 読めなかったら止める
	assert(SUCCEEDED(hr_));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知
	// 2. Compileする
	LPCWSTR arguments[] = {
	    filePath.c_str(), // コンパイル対象のhlslファイル名
	    L"-E",
	    L"main", // エントリーポイントの指定。基本的にmain以外は指定しない
	    L"-T",
	    profile, // ShaderProfileの設定
	    L"-Zi",
	    L"-Qembed_debug", // デバッグ用の情報を埋め込む
	    L"-Od",           // 最適化を外しておく
	    L"-Zpr",          // メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr_ = dxcCompiler_->Compile(
	    &shaderSourceBuffer,        // 読み込んだファイル
	    arguments,                  // コンパイルオプション
	    _countof(arguments),        // コンパイルオプションの数
	    includeHandler_,             // includeが含まれた場合
	    IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr_));
	// 3. 警告・エラーがでていないか確認する
	// // 警告・エラーが出たらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
		assert(false);
	}
	// 4. Compile結果を受け取って返す
	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr_ = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr_));

	// 成功したログを出す
	Log(CStr_.ConvertString_(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();

	// 実行用のバイナリを返却
	return shaderBlob;
}

void GameBase::PSO() {
	Log("PSO() Start\n");
	assert(device_ != nullptr);
	Log("device_ is OK\n");

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter作成。Material(PixelShader用)とTransform(VertexShader用)
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 0番目: PixelShader用のMaterial
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// 1番目: VertexShader用のTransform
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 1;


	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	//Sampler
	D3D12_STATIC_SAMPLER_DESC staticSampler[1] = {};
	staticSampler[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler[0].ShaderRegister = 0;
	staticSampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers=staticSampler;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSampler);

	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	hr_ = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	rootSignature_ = nullptr;
	hr_ = device_->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	

	assert(SUCCEEDED(hr_));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};

	// POSITION（float4）
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].InputSlot = 0;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[0].InstanceDataStepRate = 0;

	// TEXCOORD（float2）
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].InputSlot = 0;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[1].InstanceDataStepRate = 0;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

// BlendStateの設定（透過を有効化）
	D3D12_BLEND_DESC blendDesc{};
	auto& rt = blendDesc.RenderTarget[0];
	rt.BlendEnable = TRUE;                    // ブレンド有効化
	rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;      // ソース：アルファ
	rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // デスティネーション：1-ソースアルファ
	rt.BlendOp = D3D12_BLEND_OP_ADD;          // 加算
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;       // アルファチャンネル用（通常はONE）
	rt.DestBlendAlpha = D3D12_BLEND_ZERO;     // アルファ
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;     // アルファも加算
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	 vertexShaderBlob_ = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(pixelShaderBlob_ != nullptr);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_;                                                 // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                  // InputLayout
	graphicsPipelineStateDesc.VS = {vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize()};   // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                         // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                               // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// DepthStencil の設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;                           // Depth 有効化
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  // 書き込みON
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 近いほど前に表示

	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // DSV のフォーマットを指定


	// 利用するポリゴン（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 実際に生成
	graphicsPipelineState_ = nullptr;
	hr_ = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr_));
}

void GameBase::VertexResource() {
	// 頂点リソース作成
	vertexResource_ = CreateBufferResource(device_, sizeof(VertexData) * kMaxVertices_);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * kMaxVertices_;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	
	

	// ビューポートとシザー設定
	viewport_ = {};
	viewport_.Width = float(kClientWidth_);
	viewport_.Height = float(kClientHeight_);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	scissorRect_ = {};
	scissorRect_.left = 0;
	scissorRect_.right = kClientWidth_;
	scissorRect_.top = 0;
	scissorRect_.bottom = kClientHeight_;

	// --- マテリアル用リソース ---
	materialResource_ = CreateBufferResource(device_, sizeof(Vector4));
	Vector4* materialData = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 赤
	materialResource_->Unmap(0, nullptr);
	// --- トランスフォーム用リソース ---
	transformResource_ = CreateBufferResource(device_, sizeof(Matrix4x4));
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_)); // ←ここ！！起動時にマップしっぱなし
	*transformationMatrixData_ = function_.MakeIdentity(); // 初期値は単位行列]// 例：Drawの最後に Unmap する
	
	Matrix4x4 worldMatrix = function_.MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	Matrix4x4 cameraMatrix = function_.MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	Matrix4x4 viewMatrix = function_.Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = function_.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = function_.Multiply(worldMatrix, function_.Multiply(viewMatrix, projectionMatrix));
	
	*transformationMatrixData_ = worldViewProjectionMatrix;

}



ID3D12Resource* GameBase::CreateBufferResource(ID3D12Device* device_, size_t sizeInBytes) {
	// バッファの設定（UPLOAD用に変更）
	D3D12_HEAP_PROPERTIES heapProperties_ = {};
	heapProperties_.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc_ = {};
	resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc_.Width = sizeInBytes;
	resourceDesc_.Height = 1;
	resourceDesc_.DepthOrArraySize = 1;
	resourceDesc_.MipLevels = 1;
	resourceDesc_.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc_.SampleDesc.Count = 1;
	resourceDesc_.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc_.Flags = D3D12_RESOURCE_FLAG_NONE;

	bufferResource_ = nullptr;

	HRESULT hr_ = device_->CreateCommittedResource(
	    &heapProperties_, D3D12_HEAP_FLAG_NONE, &resourceDesc_,
	    D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadならこれ
	    nullptr, IID_PPV_ARGS(&bufferResource_));

	if (FAILED(hr_)) {
		return nullptr;
	}

	return bufferResource_;
}

void GameBase::Update() {
	// --- 回転角度を更新（Y軸回転だけ）
	transform_.rotate.y += 0.03f;

	// --- ワールド行列を作成（スケール → 回転 → 移動）
	Matrix4x4 worldMatrix = function_.MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	Matrix4x4 cameraMatrix = function_.MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	Matrix4x4 viewMatrix = function_.Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = function_.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = function_.Multiply(worldMatrix, function_.Multiply(viewMatrix, projectionMatrix));
	*transformationMatrixData_ = worldViewProjectionMatrix;

	// --- バッファに書き込む
	/**wvpData = worldMatrix;*/
}

//void GameBase::Draw() {
//	// ★ここ！毎回リセットする
//	hr_ = commandAllocator_->Reset();
//	assert(SUCCEEDED(hr_));
//	hr_ = commandList_->Reset(commandAllocator_, nullptr);
//	assert(SUCCEEDED(hr_));
//
//	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
//
//	DrawCommandList();
//
//	
//	
//
//	//ImGui::ShowDemoWindow();
//
//	// ImGui 描画（SRVヒープとコマンドリストを渡す）
//	imguiM_.Render(srvDescriptorHeap_, commandList_);
//
//	// RenderTarget → Present に戻す
//	
//	CrtvTransitionBarrier();
//
//	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
//	hr_ = commandList_->Close();
//
//	assert(SUCCEEDED(hr_));
//
//	// GPUにコマンドリストの実行を行わせる
//	ID3D12CommandList* commandLists[] = {commandList_};
//	commandQueue_->ExecuteCommandLists(1, commandLists);
//	// GPUと05に画面の交換を行うよう通知する
//	// GPUとOSに画面の交換を行うよう通知する
//	swapChain_->Present(1, 0);
//
//	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
//
//
//	// Fenceで同期
//	fenceValue_++;
//	commandQueue_->Signal(fence_, fenceValue_);
//
//	if (fence_->GetCompletedValue() < fenceValue_) {
//		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
//		WaitForSingleObject(fenceEvent_, INFINITE);
//	}
//
//}
//
//

void GameBase::FrameStart() {
	hr_ = commandAllocator_->Reset();
	assert(SUCCEEDED(hr_));
	hr_ = commandList_->Reset(commandAllocator_, nullptr);
	assert(SUCCEEDED(hr_));
}

ID3D12DescriptorHeap* GameBase::CreateDescriptorHeap(ID3D12Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr_ = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr_));

	return descriptorHeap;
}
ID3D12Resource* GameBase::CreateDepthStencilTextureResource(ID3D12Device* device_, int32_t width, int32_t height) {

	D3D12_RESOURCE_DESC resourceDesc_{};
	resourceDesc_.Width = width;
	resourceDesc_.Height = height;
	resourceDesc_.MipLevels = 1;
	resourceDesc_.DepthOrArraySize = 1;
	resourceDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc_.SampleDesc.Count = 1;
	resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc_.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties_{};
	heapProperties_.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	resource_ = nullptr;
	hr_ = device_->CreateCommittedResource(&heapProperties_, D3D12_HEAP_FLAG_NONE, &resourceDesc_, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr_));

	
	return resource_;


}

void GameBase::DrawCommandList() {


	


	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックアップに対して行う
	barrier_.Transition.pResource = swapChainResources_[backBufferIndex_];
	// 遷移前(現在)のResourceState
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	assert(commandList_ != nullptr);

	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);

	// 描画先のRTVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex_], false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f}; // 青っぽい色。RGBAの順
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex_], clearColor, 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList_->RSSetViewports(1, &viewport_);       // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_); // Scissorを設定

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList_->SetGraphicsRootSignature(rootSignature_);
	commandList_->SetPipelineState(graphicsPipelineState_);                                         // PSOを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);                                     // VBVを設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());  // PixelShader側
	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress()); // VertexShader側
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap_};
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);

	commandList_->SetGraphicsRootDescriptorTable(2, GPUHandle_);

	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
	commandList_->DrawInstanced(6, 1, 0, 0);





}

// （その他インクルードは省略）

// --- フレーム開始: Reset → バックバッファ取得 → RenderTarget設定＆クリア ---
// GameBase.cpp より抜粋
void GameBase::BeginFlame() {
	// ① 現在のバックバッファをフレーム毎に更新
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// --- 安全チェック ---
	assert(backBufferIndex_ < 2);
	assert(swapChainResources_[backBufferIndex_] != nullptr); // 安全強化！

	// ② 頂点オフセットリセット
	currentVertexOffset_ = 0;

	// ③ コマンドリストのリセット
	FrameStart();

	// ④ バックバッファへのバリア & RTV 設定 & クリア
	DrawCommandList();

	// ⑤ ImGui 準備
	imguiM_.NewFrame();
}




// --- フレーム終了: ImGui 描画 → Present → フェンス同期まで ---
void GameBase::EndFlame() {
	
	imguiM_.Render(srvDescriptorHeap_, commandList_);

	// RenderTarget→Present に戻す
	CrtvTransitionBarrier(); // バリア遷移 :contentReference[oaicite:4]{index=4}:contentReference[oaicite:5]{index=5}

	// コマンドリストをクローズして実行
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));
	ID3D12CommandList* lists[] = {commandList_};
	commandQueue_->ExecuteCommandLists(1, lists);

	// 画面を切り替え
	swapChain_->Present(1, 0);

	// フェンスで CPU/GPU 同期
	fenceValue_++;
	commandQueue_->Signal(fence_, fenceValue_);
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}


void GameBase::DrawTriangle(const Vector3 positions[3], const Vector2 texcoords[3], const Vector4& color, Texture& texture) {
	// オフセットを今の位置で取得
	UINT offsetVerts = currentVertexOffset_;
	// 1) 頂点バッファをマップして、offset から書き込み
	VertexData* vd = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vd));
	vd += offsetVerts; // ← 3 頂点単位でずらす
	for (int i = 0; i < 3; ++i) {
		vd[i].position = {positions[i].x, positions[i].y, positions[i].z, 1.0f};
		vd[i].texcoord = texcoords[i];
	}
	vertexResource_->Unmap(0, nullptr);

	// 2) マテリアルカラーを更新
	Vector4* mat = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat));
	*mat = color;
	materialResource_->Unmap(0, nullptr);

	// 3) ワールドビュー射影行列は既に transformResource に書き込まれている前提

	// 4) ルートパラメータ設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());

	// 5) テクスチャ用ディスクリプタ
	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_};
	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);
	commandList_->SetGraphicsRootDescriptorTable(2, texture.GetGpuHandle());

	// 6) 頂点バッファビューを設定
	// 頂点バッファビューを設定（オフセットを反映）
	
	D3D12_VERTEX_BUFFER_VIEW vbv{};
	vbv.BufferLocation = +vertexResource_->GetGPUVirtualAddress() + +offsetVerts * sizeof(VertexData);
	vbv.SizeInBytes = sizeof(VertexData) * 3;
	vbv.StrideInBytes = sizeof(VertexData);
	commandList_->IASetVertexBuffers(0, 1, &vbv);

	// 7) プリミティブ／ドロー
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->DrawInstanced(3, 1, 0, 0);

	
  // ⑧ 次回のオフセットを進める
	currentVertexOffset_ += 3;
}

int GameBase::LoadTexture(const std::string& fileName) {
	// 新しいTextureオブジェクトをvectorに追加
	textures_.emplace_back();
	// Initializeのオーバーロードを利用してファイル名で読み込み
	textures_.back().Initialize(device_, srvDescriptorHeap_, fileName);
	return static_cast<int>(textures_.size() - 1);
}


// GameBase.cpp の適当な場所に追加
void GameBase::SetWorldViewProjection(const Matrix4x4& wvp) { *transformationMatrixData_ = wvp; }
