#define NOMINMAX
#include "GameBase.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

GameBase::~GameBase(){

	
	ResourceRelease();

}


LRESULT CALLBACK GameBase::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}


void GameBase::Log(const std::string& message) {	

	OutputDebugStringA(message.c_str()); 
}

void GameBase::Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight) {

	wc.lpfnWndProc = WindowProc;

	wc.lpszClassName = L"CG2WindowClass";

	wc.hInstance = GetModuleHandle(nullptr);

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);


	wrc = {0, 0, WindowWidth, WindowHeight};

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(wc.lpszClassName, TitleName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);
	///Debuglayer
	DebugLayer();

	ShowWindow(hwnd, SW_SHOW);

	// DXGIファクトリーの生成
	dxgiFactory = nullptr;
	// HRESULTはWindows系のエラーコードであり、
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる

	hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr_));

	
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr_ = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr_)); // 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			Log(CStr->ConvertString_(std::format(L"Use Adapater: {}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかった。
	}
	// 適切なアダプタが見つからなかったので起動できない。
	assert(useAdapter != nullptr);

	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {

	    D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		// 採用したアダプターでデバイスを生成
		hr_ = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));

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
	
	CreateResource();

}

bool GameBase::IsMsgQuit() {

	if (msg.message != WM_QUIT) {
	
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
	Log(CStr->ConvertString_(std::format(L"WSTRING {}\n", wstringValue)));
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
	commandQueueDesc = {};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 主に描画用途
	hr_ = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));

	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	// コマンドアロケータを生成する
	commandAllocator = nullptr;
	hr_ = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	// コマンドリストを生成する
	commandList_ = nullptr;
	hr_ = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	swapChain_ = nullptr;
	swapChainDesc = {};

	swapChainDesc.Width = kClientWidth;
	swapChainDesc.Height = kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する

	hr_ = dxgiFactory->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr_));

	// ディスクリプタヒープの生成

	rtvDescriptorHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	
	// SRV用ディスクリプタヒープ作成
	srvDescriptorHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);



	//rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
	//rtvDescriptorHeapDesc.NumDescriptors = 2;                    // ダブルバッファ用に2つ。多くてもかまわない
	//hr_ = device_->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	//// ディスクリプタヒープがつくれなかったので起動できない
	//assert(SUCCEEDED(hr_));
	DXCInitialize();
	DInput.Initialize(wc,hwnd);
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
	rtvHandles[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc, rtvHandles[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc, rtvHandles[1]);

	depthStenicilResource = CreateDepthStencilTextureResource(device_.Get(), kClientWidth, kClientHeight);

	dsvDescriptorHeap = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device_->CreateDepthStencilView(depthStenicilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());



	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// ImGui 初期化はここで！
	imguiM_.MInitialize(hwnd, device_.Get(), swapChainDesc, rtvDesc, srvDescriptorHeap_.Get());
	
	if (srvDescriptorHeap_ == nullptr) {
		assert(false);
	}



}

void GameBase::CreateResource(){


	DrawCommandList();

	CrtvTransitionBarrier();

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr_ = commandList_->Close();

	assert(SUCCEEDED(hr_));

	// GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, commandLists->GetAddressOf());
	// GPUと05に画面の交換を行うよう通知する
	// GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);

	// Fenceを作る
	fenceValue_ = 0;
	hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ != nullptr);

	// Fenceの値を更新
	fenceValue_++;
	commandQueue_->Signal(fence_.Get(), fenceValue_);

	// Fenceの値が指定Signalに達してるか確認
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE); // ★ここでちゃんとGPUが終わるまで待つ！
	}

	audio.InitializeIXAudio();

	
	HRESULT hr = DirectInput8Create(
	    wc.hInstance,        // WinMain から渡した HINSTANCE
	    DIRECTINPUT_VERSION, // DirectInput のバージョン
	    IID_IDirectInput8,   // ← ここを使う
	    reinterpret_cast<void**>(directInput_.GetAddressOf()), nullptr);
	assert(SUCCEEDED(hr));

	InitializeMouse(hwnd);
}

void GameBase::DebugLayer() {

#ifdef NDEBUG



	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
	//デバッグレイヤー
		debugController->EnableDebugLayer();
		//更にGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
	

#endif // DEBUG

}

void GameBase::DebugError() {

#ifdef NDEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
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
		infoQueue.Reset();

	
	}


#endif // _DEBUG



}

void GameBase::TransitionBarrier() {
	//transitionBarrierの設定
	
	


}

void GameBase::CrtvTransitionBarrier() {
	//画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	//今回はRenderTargetからPresentにする。
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);
}

void GameBase::FenceEvent() {
	// Fenceの作成
	hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));

	// ←ここ追加！！
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ != nullptr);
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
	
	//texture_.Finalize();
	//texture2_.Finalize();
	imguiM_.Finalize();

	
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
	if (transformResource_) {
		transformResource_->Unmap(0, nullptr); // ちゃんと最後だけunmapする
	}
	if (directionalLightResource_) {
		directionalLightResource_->Unmap(0, nullptr);
		/*directionalLightResource_->Release();*/
		directionalLightResource_ = nullptr;
		directionalLightData_ = nullptr;
	}
	

	CloseWindow(hwnd);
}


void GameBase::DXCInitialize() {

	// dxcCompilerを初期化
	dxcUtils = nullptr;
	dxcCompiler = nullptr;
	hr_ = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr_));
	hr_ = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr_));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler = nullptr;
	hr_ = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr_));

	SetupPSO();
	VertexResource();

}

Microsoft::WRL::ComPtr<IDxcBlob> GameBase::CompileShader(/* CompilerするShaderファイルへのパス*/const std::wstring& filePath,
    // Compilerに使用するProfile
    const wchar_t* profile,
    // 初期化で生成したものをつかう
    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	// ここの中身をこの後書いていく
	// 1. hlslファイルを読む
	// // これからシェーダーをコンパイルする旨をログに出す
	Log(CStr->ConvertString_(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読む
	Microsoft::WRL::ComPtr < IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr_ = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

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
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr_ = dxcCompiler->Compile(
	    &shaderSourceBuffer,        // 読み込んだファイル
	    arguments,                  // コンパイルオプション
	    _countof(arguments),        // コンパイルオプションの数
	    includeHandler,             // includeが含まれた場合
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
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr_ = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr_));

	// 成功したログを出す
	Log(CStr->ConvertString_(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// もう使わないリソースを解放
	shaderSource.Reset();
	shaderResult.Reset();

	// 実行用のバイナリを返却
	return shaderBlob;
}
void GameBase::SetupPSO() {
	Log("SetupPSO() Start\n");
	assert(device_ != nullptr);

	// --- RootSignature ---
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 1;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 3;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	D3D12_STATIC_SAMPLER_DESC staticSampler[1] = {};
	staticSampler[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler[0].ShaderRegister = 0;
	staticSampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSampler;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSampler);

	// Lightバッファ
	directionalLightResource_ = CreateBufferResource(device_.Get(), sizeof(DirectionalLight));
	assert(directionalLightResource_);
	directionalLightData_ = nullptr;
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);
	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };

	// --- RootSignature作成 ---
	signatureBlob = nullptr;
	errorBlob = nullptr;
	hr_ = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr_)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	rootSignature = nullptr;
	hr_ = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr_));

	// --- InputLayout ---
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].InputSlot = 0;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[0].InstanceDataStepRate = 0;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].InputSlot = 0;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[1].InstanceDataStepRate = 0;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// --- Blend ---
	D3D12_BLEND_DESC blendDesc{};
	auto& rtBlend = blendDesc.RenderTarget[0];
	rtBlend.BlendEnable = FALSE;
	rtBlend.LogicOpEnable = FALSE;
	rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// --- DepthStencil ---
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// --- 共通設定 ---
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc;
	baseDesc.BlendState = blendDesc;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.DepthStencilState = depthStencilDesc;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// --- 通常PSO（裏面カリング） ---
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = CompileShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
	Microsoft::WRL::ComPtr<IDxcBlob> psBlob = CompileShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
	assert(vsBlob && psBlob);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = baseDesc;
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK /*D3D12_CULL_MODE_NONE*/; // 裏面カリング
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState = rasterizerDesc;
	hr_ = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipelineState[PSO::BlendMode::kBlendModeAlpha]));
	assert(SUCCEEDED(hr_));

	// --- メタボール用PSO（両面描画＋真っ白PS） ---
	Microsoft::WRL::ComPtr<IDxcBlob> whitePSBlob = CompileShader(L"Resources/shader/WhitePS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
	assert(whitePSBlob);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC metaballPsoDesc = baseDesc;
	metaballPsoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	metaballPsoDesc.PS = {whitePSBlob->GetBufferPointer(), whitePSBlob->GetBufferSize()};
	D3D12_RASTERIZER_DESC rasterizerDescMetaball{};
	rasterizerDescMetaball.CullMode = D3D12_CULL_MODE_NONE; // 両面描画
	rasterizerDescMetaball.FillMode = D3D12_FILL_MODE_SOLID;
	metaballPsoDesc.RasterizerState = rasterizerDescMetaball;
	hr_ = device_->CreateGraphicsPipelineState(&metaballPsoDesc, IID_PPV_ARGS(&graphicsPipelineStateWhite));
	assert(SUCCEEDED(hr_));

	Log("SetupPSO END \n");
}



void GameBase::VertexResource() {
	Log("VertexResource Start\n");
	// 頂点リソース作成

	modelData = LoadObjFile("Resources/3d", "plane.obj");

	vertexResource_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * modelData.vertices.size());
	
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0,nullptr,reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// ビューポートとシザー設定
	viewport = {};
	viewport.Width = float(kClientWidth);
	viewport.Height = float(kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect = {};
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	// --- マテリアル用リソース ---
	// 3D用（球など陰影つけたいもの）
	// 必ず256バイト単位で切り上げる
	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = CreateBufferResource(device_.Get(), alignedSize);
	Material* mat3d = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mat3d->enableLighting = 1;
	mat3d->uvTransform = function.MakeIdentity();

	materialResource_->Unmap(0, nullptr);

	// --- トランスフォーム用リソース ---
	// 配列サイズ: [0]=モデル/デバッグ, [1]=未使用, [2]～=球体用
	
	constexpr UINT kCBAlign = 256;
	UINT matrixAlignedSize = (sizeof(TransformationMatrix) + kCBAlign - 1) & ~(kCBAlign - 1);
	transformResource_ = CreateBufferResource(device_.Get(), matrixAlignedSize * kMaxTransformSlots);

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData)); // 構造体配列として使う

	// 初期化は必ず構造体単位で
	for (int i = 0; i < kMaxTransformSlots; ++i) {
		transformationMatrixData[i].WVP = function.MakeIdentity();
		transformationMatrixData[i].World = function.MakeIdentity();
	}

	// [0]=モデル描画用で使う
	Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData[0].WVP = worldViewProjectionMatrix;
	transformationMatrixData[0].World = worldMatrix;



	// --- Sprite用 頂点リソース ---
	vertexResourceSprite_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * kMaxSpriteVertices);
	vertexBufferViewSprite={};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite_->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * kMaxSpriteVertices;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	// --- ここにインデックスバッファの生成を追加 ---
	// --- ここにインデックスバッファの生成を追加 ---
	// 6個のインデックス（2枚の三角形でスプライト）
	indexResourceSprite_ = CreateBufferResource(device_.Get(), sizeof(uint32_t) * 6);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

// 1枚目の三角形（左下 → 左上 → 右下）
	vertexDataSprite[0].position = {0.0f, 360.0f, 0.0f, 1.0f}; // 左下
	vertexDataSprite[0].texcoord = {0.0f, 1.0f};
	

	vertexDataSprite[1].position = {0.0f, 0.0f, 0.0f, 1.0f}; // 左上
	vertexDataSprite[1].texcoord = {0.0f, 0.0f};

	vertexDataSprite[2].position = {640.0f, 360.0f, 0.0f, 1.0f}; // 右下
	vertexDataSprite[2].texcoord = {1.0f, 1.0f};

	// 2枚目の三角形（左上 → 右上 → 右下）
	vertexDataSprite[3].position = {0.0f, 0.0f, 0.0f, 1.0f}; // 左上
	vertexDataSprite[3].texcoord = {0.0f, 0.0f};

	vertexDataSprite[4].position = {640.0f, 0.0f, 0.0f, 1.0f}; // 右上
	vertexDataSprite[4].texcoord = {1.0f, 0.0f};

	vertexDataSprite[5].position = {640.0f, 360.0f, 0.0f, 1.0f}; // 右下
	vertexDataSprite[5].texcoord = {1.0f, 1.0f};

	for (int i = 0; i < 6; i++) {
		vertexDataSprite[i].normal = {0.0f, 0.0f, -1.0f};
	}

	vertexResourceSprite_->Unmap(0, nullptr);
	// インデックスバッファ作成部分
	// 四角形（クワッド）を2枚の三角形に分割
	uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
	// この内容で indexResourceSprite_ を初期化してください


	void* mapped = nullptr;
	indexResourceSprite_->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, sizeof(indices));
	indexResourceSprite_->Unmap(0, nullptr);

	// インデックスバッファビューの作成
	indexBufferViewSprite_.BufferLocation = indexResourceSprite_->GetGPUVirtualAddress();
	indexBufferViewSprite_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewSprite_.Format = DXGI_FORMAT_R32_UINT;
	
	


// スプライト用（陰影つけたくないもの）
	materialResourceSprite_ = CreateBufferResource(device_.Get(), alignedSize);
	Material* matSprite = nullptr;
	materialResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&matSprite));
	matSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白 or テクスチャの色
	matSprite->enableLighting = 0;
	matSprite->uvTransform = function.MakeIdentity();
	materialResourceSprite_->Unmap(0, nullptr);


	// Sprite用の TransformationMatrix リソース作成（1個分）
	transformationMatrixResourceSprite_ = CreateBufferResource(device_.Get(), sizeof(Matrix4x4) * 2);

	// データへのポインタ取得
	transformationMatrixDataSprite = nullptr;
	transformationMatrixResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

	// 単位行列を書き込んでおく（初期状態）
	*transformationMatrixDataSprite = function.MakeIdentity();

	


		// 球体メッシュの追加（新規追加）
		const int kSubdivision = 16;
		const float pi = 3.14159265f;
		const float kLonEvery = pi * 2.0f / float(kSubdivision);
		const float kLatEvery = pi / float(kSubdivision);

		const int kVertexCount = kSubdivision * kSubdivision * 6;
	    kVertexCount_ = kVertexCount;
	    vertexBufferViewSphere = {};
	    vertexResourceSphere = CreateBufferResource(device_.Get(), sizeof(VertexData) * kVertexCount);
		vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
		vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * kVertexCount;
		vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);

		VertexData* sphereVertexData = nullptr;
		vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&sphereVertexData));

		for (int latIndex = 0; latIndex < kSubdivision; ++latIndex) {
			float lat = -pi / 2.0f + kLatEvery * latIndex;
			for (int lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
				float lon = lonIndex * kLonEvery;
				int start = (latIndex * kSubdivision + lonIndex) * 6;

				float nextLat = lat + kLatEvery;
				float nextLon = lon + kLonEvery;

				Vector4 a = {cosf(lat) * cosf(lon), sinf(lat), cosf(lat) * sinf(lon), 1.0f};
				Vector4 b = {cosf(nextLat) * cosf(lon), sinf(nextLat), cosf(nextLat) * sinf(lon), 1.0f};
				Vector4 c = {cosf(lat) * cosf(nextLon), sinf(lat), cosf(lat) * sinf(nextLon), 1.0f};
				Vector4 d = {cosf(nextLat) * cosf(nextLon), sinf(nextLat), cosf(nextLat) * sinf(nextLon), 1.0f};

				float u = float(lonIndex) / float(kSubdivision);
				float v = 1.0f - float(latIndex) / float(kSubdivision);

				// 上の球体生成for文の直前に追加
			    auto calcNormal = [](const Vector4& v) -> Vector3 {
				    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
				    if (len == 0)
					    return {0, 0, 1};
				    return {v.x / len, v.y / len, v.z / len};
			    };

			    // ループ内
			    sphereVertexData[start + 0] = {
			        a, {u, v},
                     calcNormal(a)
                };
			    sphereVertexData[start + 1] = {
			        b, {u, v - (1.0f / kSubdivision)},
                     calcNormal(b)
                };
			    sphereVertexData[start + 2] = {
			        c, {u + (1.0f / kSubdivision), v},
                     calcNormal(c)
                };
			    sphereVertexData[start + 3] = {
			        c, {u + (1.0f / kSubdivision), v},
                     calcNormal(c)
                };
			    sphereVertexData[start + 4] = {
			        b, {u, v - (1.0f / kSubdivision)},
                     calcNormal(b)
                };
			    sphereVertexData[start + 5] = {
			        d, {u + (1.0f / kSubdivision), v - (1.0f / kSubdivision)},
                     calcNormal(d)
                };

			}
		}
	    // 球体メッシュの範囲を出力
	    float minX = 9999, maxX = -9999, minY = 9999, maxY = -9999, minZ = 9999, maxZ = -9999;
	    for (int i = 0; i < kVertexCount; ++i) {
		    minX = std::min(minX, sphereVertexData[i].position.x);
		    maxX = std::max(maxX, sphereVertexData[i].position.x);
		    minY = std::min(minY, sphereVertexData[i].position.y);
		    maxY = std::max(maxY, sphereVertexData[i].position.y);
		    minZ = std::min(minZ, sphereVertexData[i].position.z);
		    maxZ = std::max(maxZ, sphereVertexData[i].position.z);
	    }
	    OutputDebugStringA(std::format("Sphere mesh range X:{:.2f}~{:.2f} Y:{:.2f}~{:.2f} Z:{:.2f}~{:.2f}\n", minX, maxX, minY, maxY, minZ, maxZ).c_str());

		vertexResourceSphere->Unmap(0, nullptr);
	    Log("VertexResource END\n");
}
Microsoft::WRL::ComPtr <ID3D12Resource> GameBase::CreateBufferResource(ID3D12Device* device_, size_t sizeInBytes) {
	// バッファの設定（UPLOAD用に変更）
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;

	HRESULT hr_ = device_->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	    D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadならこれ
	    nullptr, IID_PPV_ARGS(&bufferResource));

	if (FAILED(hr_)) {
		return nullptr;
	}

	return bufferResource;
}
void GameBase::CreateModelVertexBuffer() {
	Log("VertexResource Start\n");
	// 頂点リソース作成

	

	const int MaxVertexCount = 655360;
	vertexResource_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * MaxVertexCount);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// ビューポートとシザー設定
	viewport = {};
	viewport.Width = float(kClientWidth);
	viewport.Height = float(kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect = {};
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	// --- マテリアル用リソース ---
	// 3D用（球など陰影つけたいもの）
	// 必ず256バイト単位で切り上げる
	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = CreateBufferResource(device_.Get(), alignedSize);
	Material* mat3d = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mat3d->enableLighting = 1;
	mat3d->uvTransform = function.MakeIdentity();

	materialResource_->Unmap(0, nullptr);

	// --- トランスフォーム用リソース ---
	
constexpr UINT kCBAlign = 256;
	UINT matrixAlignedSize = (sizeof(TransformationMatrix) + kCBAlign - 1) & ~(kCBAlign - 1);
	transformResource_ = CreateBufferResource(device_.Get(), matrixAlignedSize * kMaxTransformSlots);

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData)); // 構造体配列で取得
	                                                                                          // ←ここ！！起動時にマップしっぱなし
	                                                                                          // すべて単位行列で初期化（任意）
	for (int i = 0; i < kMaxTransformSlots; ++i) {
		transformationMatrixData[i].WVP = function.MakeIdentity();
		transformationMatrixData[i].World = function.MakeIdentity();
	} // 初期値は単位行列
	Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
	// ProjectionMatrixの作成（FOV: 0.45f * π、アスペクト比: 16:9、近クリップ: 0.1f、遠クリップ: 1000.0f）
	float fovY = 0.45f * 3.14159265f; // ラジアン
	float aspectRatio = 16.0f / 9.0f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;

	Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
	// 2個分書き込む
	//transformationMatrixData[0].WVP = worldViewProjectionMatrix; // WVP
	//transformationMatrixData[1].World = worldMatrix;               // World
	//transformationMatrixData[1].WVP = worldViewProjectionMatrix; // WVP
	//transformationMatrixData[2].World = worldMatrix;             // World
}


void GameBase::Update() {
	
	 ImGui::Checkbox("useMonsterBall", &useMonsterBall_);

	 ImGui::DragFloat2("UVTranslate", &uvTransformSprite_.translate.x, 0.01f, -10.0f, 10.0f);
	 ImGui::DragFloat2("UVScale", &uvTransformSprite_.scale.x, 0.01f, -10.0f, 10.0f);
	 ImGui::SliderAngle("UVRotate", &uvTransformSprite_.rotate.z);


	 Matrix4x4 uvTransformMatrix = function.MakeIdentity();
	 uvTransformMatrix = function.MakeScaleMatrix(uvTransformSprite_.scale);
	 uvTransformMatrix = function.Multiply(uvTransformMatrix, function.MakeRotateZMatrix(uvTransformSprite_.rotate.z));
	 uvTransformMatrix = function.Multiply(uvTransformMatrix, function.MakeTranslateMatrix(uvTransformSprite_.translate));
	 materialDataSprite_.uvTransform = uvTransformMatrix;

	 

	materialDataSprite_.color = {1.0f, 1.0f, 1.0f, 1.0f}; // 赤で描画
	 Material* matSprite = nullptr;
	 materialResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&matSprite));
	 *matSprite = materialDataSprite_;
	 materialResourceSprite_->Unmap(0, nullptr);



	// --- 回転角度を更新（Y軸回転だけ）
	/*transform.rotate.y += 0.03f;*/

	//// --- ワールド行列を作成（スケール → 回転 → 移動）
	 Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	//Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	//Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
	//float fovY = 0.45f * 3.14159265f; // ラジアン
	//float aspectRatio = 16.0f / 9.0f;
	//float nearClip = 0.1f;
	//float farClip = 1000.0f;

	//Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	//Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
	//transformationMatrixData[0].WVP = worldViewProjectionMatrix;
	//transformationMatrixData[1].WVP = worldViewProjectionMatrix;
	//transformationMatrixData[0].World = worldMatrix;

	//// Sprite用のワールド行列（スケール・回転・移動から生成）
	//Matrix4x4 worldMatrixSprite = function.MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);

	//// View行列（カメラ、今回は単位行列で固定）
	//Matrix4x4 viewMatrixSprite = function.MakeIdentity();

	//// 射影行列（平行投影、画面サイズで生成）
	//Matrix4x4 projectionMatrixSprite = function.MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);

	//// 最終WVP行列の計算と書き込み
	//Matrix4x4 worldViewProjectionMatrixSprite = function.Multiply(worldMatrixSprite, function.Multiply(viewMatrixSprite, projectionMatrixSprite));

	//transformationMatrixDataSprite[0] = worldViewProjectionMatrixSprite;
	//transformationMatrixDataSprite[1] = worldMatrix;
	
}


void GameBase::FrameStart() {
	Log("Frame START");
	hr_ = commandAllocator->Reset();
	assert(SUCCEEDED(hr_));
	hr_ = commandList_->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr_));
}
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GameBase::CreateDescriptorHeap(ID3D12Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr_ = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr_));

	return descriptorHeap;
}
Microsoft::WRL::ComPtr <ID3D12Resource> GameBase::CreateDepthStencilTextureResource(ID3D12Device* device_, int32_t width, int32_t height) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr_ = device_->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr_));

	
	return resource;


}

void GameBase::DrawCommandList() {
	
	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックアップに対して行う
	barrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	assert(commandList_ != nullptr);

	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	// 描画先のRTVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &rtvHandles[backBufferIndex_], false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f}; // 青っぽい色。RGBAの順
	commandList_->ClearRenderTargetView(rtvHandles[backBufferIndex_], clearColor, 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList_->RSSetViewports(1, &viewport);       // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect); // Scissorを設定

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList_->SetGraphicsRootSignature(rootSignature.Get());
	commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get());                                  // PSOを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);                                     // VBVを設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());  // PixelShader側
	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress()); // VertexShader側
	commandList_->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());


	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> descriptorHeaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

}
void GameBase::BeginFlame(char* keys,char*preKeys) {
	

	// ① 現在のバックバッファをフレーム毎に更新
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// --- 安全チェック ---
	assert(backBufferIndex_ < 2);
	assert(swapChainResources_[backBufferIndex_] != nullptr); // 安全強化！

	// ② 頂点オフセットリセット
	currentTriangleVertexOffset_ = 0;
	currentSpriteVertexOffset_ = 0;
	currentSphereVertexOffset_ = 0;
	sphereDrawCallCount_ = 0;


	// ③ コマンドリストのリセット
	FrameStart();

	// ④ バックバッファへのバリア & RTV 設定 & クリア
	DrawCommandList();

	// ⑤ ImGui 準備
	imguiM_.NewFrame();
	DInput.Update((uint8_t*)keys, (uint8_t*)preKeys);
}

// --- フレーム終了: ImGui 描画 → Present → フェンス同期まで ---
void GameBase::EndFlame() {

	imguiM_.Render(srvDescriptorHeap_.Get(), commandList_.Get());

	// RenderTarget→Present に戻す
	CrtvTransitionBarrier(); // バリア遷移 :contentReference[oaicite:4]{index=4}:contentReference[oaicite:5]{index=5}

	// コマンドリストをクローズして実行
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));
	Microsoft::WRL::ComPtr < ID3D12CommandList> lists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, lists->GetAddressOf());

	// 画面を切り替え
	swapChain_->Present(1, 0);

	// フェンスで CPU/GPU 同期
	fenceValue_++;
	commandQueue_->Signal(fence_.Get(), fenceValue_);
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}



void GameBase::DrawSpriteSheet(Vector3 pos[4], Vector2 texturePos[4], int color,int textureHandle) {
	commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get()); // 通常
	// 頂点バッファに6頂点分追記
	VertexData* vertexData = nullptr;
	vertexResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	UINT offset = currentSpriteVertexOffset_;

	// 2枚の三角形で四角形を描画（インデックス: 0,1,2, 1,3,2）
	// 0: 左上, 1: 右上, 2: 右下, 3: 左下
	vertexData[offset + 0] = {
	    {pos[0].x, pos[0].y, 0, 1},
        {texturePos[0].x, texturePos[0].y},
        {0, 0, -1}
    };
	vertexData[offset + 1] = {
	    {pos[1].x, pos[1].y, 0, 1},
        {texturePos[1].x, texturePos[1].y},
        {0, 0, -1}
    };
	vertexData[offset + 2] = {
	    {pos[2].x, pos[2].y, 0, 1},
        {texturePos[2].x, texturePos[2].y},
        {0, 0, -1}
    };
	vertexData[offset + 3] = {
	    {pos[3].x, pos[3].y, 0, 1},
        {texturePos[3].x, texturePos[3].y},
        {0, 0, -1}
    };
	// 1枚スプライト用のインデックス
	// 三角形1: 0,1,2  三角形2: 0,2,3
	vertexResourceSprite_->Unmap(0, nullptr);

	// 頂点バッファビューとインデックスバッファビューをセット（オフセット指定）
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferViewSprite;
	vbv.BufferLocation += sizeof(VertexData) * offset;
	vbv.SizeInBytes = sizeof(VertexData) * 4; // 4頂点分

	D3D12_INDEX_BUFFER_VIEW ibv = indexBufferViewSprite_;
	// インデックスバッファは使いまわしでOK（インデックス: 0,1,2, 0,2,3 など4頂点分用を用意）

	commandList_->IASetVertexBuffers(0, 1, &vbv);
	commandList_->IASetIndexBuffer(&ibv);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ヒープ、ルートパラメータ等をセット（すでにやってる場合は不要）
	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);
	commandList_->SetGraphicsRootConstantBufferView(0, materialResourceSprite_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]);

	// 描画（1スプライト分）
	commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// 次のスプライト用にオフセットを進める
	currentSpriteVertexOffset_ += 4; // 4頂点分進める
}

void GameBase::DrawSphere(
    const Vector3& center,
    float radius,
    uint32_t color,
    int textureHandle,
    const Matrix4x4& viewProj)
{
	// 0. まずここでPSO切り替え
	if (IsMetaBall_) {
		commandList_->SetPipelineState(graphicsPipelineStateWhite.Get()); // 白単色
	} else {
		commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get()); // 通常
	}
    // 1. マテリアル設定
    Material* mat = nullptr;
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat));
    mat->color = Vector4(
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        (color & 0xFF) / 255.0f,
        ((color >> 24) & 0xFF) / 255.0f);
    mat->enableLighting = 1;
    mat->uvTransform = function.MakeIdentity();
    materialResource_->Unmap(0, nullptr);

    // 2. ワールド行列とWVP
    Matrix4x4 world = function.MakeAffineMatrix(
        {radius, radius, radius},   // スケール
        {0, 0, 0},                  // 回転
        center                      // 平行移動
    );
    Matrix4x4 wvp = function.Multiply(world, viewProj);
	Log(std::format("WVP = "));
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Log(std::format("{},", wvp.m[i][j]));
		}
		Log(std::format("\n"));
	}
	
    // 3. slotを自動管理
    int slot = sphereDrawCallCount_; // 0から順に
        // 次回のためにカウンタ進める
	sphereDrawCallCount_++;
	assert(slot < kMaxTransformSlots); // slotは最大数を超えていないか？
    // 4. 定数バッファへの書き込み（slot番目に格納）
    transformationMatrixData[slot].WVP   = wvp;
    transformationMatrixData[slot].World = world;
	// WaterController.cpp のループ内
	OutputDebugStringA(std::format("Sphere #(slot){}: center=({:.2f},{:.2f},{:.2f}), r={:.2f}, viewProj[0][0]={:.2f}\n", slot, center.x, center.y, center.z, radius, viewProj.m[0][0]).c_str());


    // 5. 頂点バッファ
    D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferViewSphere;
    commandList_->IASetVertexBuffers(0, 1, &vbv);
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 6. テクスチャ
	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]);

    // 7. マテリアル
    commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // 8. 定数バッファ（球体ごとにアドレスをずらしてバインド！）
	constexpr UINT kCBAlign = 256;
	UINT matrixAlignedSize = (sizeof(TransformationMatrix) + kCBAlign - 1) & ~(kCBAlign - 1);

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = transformResource_->GetGPUVirtualAddress() + slot * matrixAlignedSize;
	commandList_->SetGraphicsRootConstantBufferView(1, cbAddress);


    // 9. ライト
    commandList_->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

    // 10. 描画
    commandList_->DrawInstanced(kVertexCount_, 1, 0, 0);
	auto& tr = transformationMatrixData[1].World;
	Log(std::format("\n World matrix scale = ({}, {}, {})\n", tr.m[0][0], tr.m[1][1], tr.m[2][2])); // 実装に合わせて要調整

}
void GameBase::DrawSphere(const Vector3& center,const Vector3& radius,const Vector3& rotation, uint32_t color, int textureHandle, const Matrix4x4& viewProj) {
	// 0. まずここでPSO切り替え
	if (IsMetaBall_) {
		commandList_->SetPipelineState(graphicsPipelineStateWhite.Get()); // 白単色
	} else {
		commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get()); // 通常
	}

	// 1. マテリアル設定
	Material* mat = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat));
	mat->color = Vector4(((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f, ((color >> 24) & 0xFF) / 255.0f);
	mat->enableLighting = 1;
	mat->uvTransform = function.MakeIdentity();
	materialResource_->Unmap(0, nullptr);

	// 2. ワールド行列とWVP
	Matrix4x4 world = function.MakeAffineMatrix(
	    radius, // スケール
	    rotation,                // 回転
	    center                    // 平行移動
	);
	Matrix4x4 wvp = function.Multiply(world, viewProj);
	Log(std::format("WVP = "));
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Log(std::format("{},", wvp.m[i][j]));
		}
		Log(std::format("\n"));
	}

	// 3. slotを自動管理
	int slot = sphereDrawCallCount_; // 0から順に
	                                 // 次回のためにカウンタ進める
	sphereDrawCallCount_++;
	assert(slot < kMaxTransformSlots); // slotは最大数を超えていないか？
	// 4. 定数バッファへの書き込み（slot番目に格納）
	transformationMatrixData[slot].WVP = wvp;
	transformationMatrixData[slot].World = world;
	// WaterController.cpp のループ内
	/*OutputDebugStringA(std::format("Sphere #(slot){}: center=({:.2f},{:.2f},{:.2f}), r={:.2f}, viewProj[0][0]={:.2f}\n", slot, center.x, center.y, center.z, radius, viewProj.m[0][0]).c_str());*/

	// 5. 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferViewSphere;
	commandList_->IASetVertexBuffers(0, 1, &vbv);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 6. テクスチャ
	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]);

	// 7. マテリアル
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// 8. 定数バッファ（球体ごとにアドレスをずらしてバインド！）
	constexpr UINT kCBAlign = 256;
	UINT matrixAlignedSize = (sizeof(TransformationMatrix) + kCBAlign - 1) & ~(kCBAlign - 1);

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = transformResource_->GetGPUVirtualAddress() + slot * matrixAlignedSize;
	commandList_->SetGraphicsRootConstantBufferView(1, cbAddress);

	// 9. ライト
	commandList_->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// 10. 描画
	commandList_->DrawInstanced(kVertexCount_, 1, 0, 0);
	auto& tr = transformationMatrixData[1].World;
	Log(std::format("\n World matrix scale = ({}, {}, {})\n", tr.m[0][0], tr.m[1][1], tr.m[2][2])); // 実装に合わせて要調整
}
//objfileを読む関数
ModelData GameBase::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			// ここでx反転
			position.x *= -1.0f;
			
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y=1.0f-texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			// ここで法線もx反転
			
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = {position, texcoord, normal};
			}
			// 回り順を逆にしてpush_back
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			std::string mtlFile;
			s >> mtlFile;
			modelData.material = LoadMaterialTemplateFile(directoryPath, mtlFile);
		}

	}

	

	return modelData;
}

MaterialData GameBase::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData matData;
	std::ifstream file(directoryPath + "/" + filename);
	std::string line;
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::istringstream s(line);
		std::string identifier;
		s >> identifier;
		if (identifier == "map_Kd") {
			std::string textureFilePaths;
			s >> textureFilePaths;
			matData.textureFilePath = directoryPath + "/" + textureFilePaths;
			
		}
	}
	return matData;
}

// ファイル名からSRVヒープ内のテクスチャindexを返す（なければ追加ロード）
//int GameBase::LoadTexture(const std::string& fileName) {
//	for (size_t i = 0; i < textures_.size(); ++i) {
//		if (textures_[i].GetFilePath() == fileName) {
//			return static_cast<int>(i); // すでにロード済み
//		}
//	}
//	// 未ロードなら新しくロードしてpush_back
//	Texture tex;
//	tex.Initialize(device_.Get(), srvDescriptorHeap_.Get(), fileName, (uint32_t)textures_.size());
//	tex.SetFilePath(fileName); // Textureクラスにファイルパス保持用メンバ追加して
//	textures_.push_back(tex);
//	OutputDebugStringA(std::format("Texture loaded: {}, GPU Handle ptr={}\n", fileName, tex.GetGpuHandle().ptr).c_str());
//	return static_cast<int>(textures_.size() - 1);
//}
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
void GameBase::InitializeMouse(HWND hwnd) {

	// directInput_ は既に DirectInput8Create で作成済み
	// マウスデバイスを作成
	HRESULT hr = directInput_->CreateDevice(GUID_SysMouse, &mouseDevice_, nullptr);
	if (FAILED(hr))
		return;

	// データフォーマットを設定（拡張マウス状態）
	hr = mouseDevice_->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr))
		return;

	// 協調レベル：フォアグラウンドかつ他アプリと共有
	hr = mouseDevice_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		return;

	// 最初の Acquire
	mouseDevice_->Acquire();
}
void GameBase::UpdateMouse() {
	// １）前フレームの状態を保存
	prevMouseState_ = mouseState_;

	// ２）DirectInput で相対移動だけ取得（マウスホイールやボタンは要るなら使う）
	if (mouseDevice_) {
		mouseDevice_->Acquire();
		mouseDevice_->GetDeviceState(sizeof(mouseState_), &mouseState_);
	}
	// （相対移動は今ここでは使わないので無視してOK）

	// ３）Windows API でマウスの絶対位置を取得
	POINT pt;
	GetCursorPos(&pt);          // スクリーン座標
	ScreenToClient(hwnd, &pt); // クライアント座標に変換
	mouseX_ = pt.x;             // ここではクランプせずそのまま代入
	mouseY_ = pt.y;
}
bool GameBase::IsMouseDown(int btn) const { return (mouseState_.rgbButtons[btn] & 0x80u) != 0; }

bool GameBase::IsMousePressed(int btn) const { return (mouseState_.rgbButtons[btn] & 0x80u) != 0 && !(prevMouseState_.rgbButtons[btn] & 0x80u); }

void GameBase::DrawMesh(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices, uint32_t color, int textureHandle) {
	//if (vertices.empty() || indices.empty())
	//	return;

	// 0. まずここでPSO切り替え
	if (IsMetaBall_) {
		commandList_->SetPipelineState(graphicsPipelineStateWhite.Get()); // 白単色
	} else {
		
		commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get()); // 通常
	}
	vertexResourceMetaball_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * vertices.size());
	vertexBufferViewMetaball_.BufferLocation = vertexResourceMetaball_->GetGPUVirtualAddress();
	vertexBufferViewMetaball_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(vertices.size());
	vertexBufferViewMetaball_.StrideInBytes = sizeof(VertexData);

	indexResourceMetaball_ = CreateBufferResource(device_.Get(), sizeof(uint32_t) * indices.size());
	indexBufferViewMetaball_.BufferLocation = indexResourceMetaball_->GetGPUVirtualAddress();
	indexBufferViewMetaball_.SizeInBytes = sizeof(uint32_t) * static_cast<UINT>(indices.size());
	indexBufferViewMetaball_.Format = DXGI_FORMAT_R32_UINT;


	/*OutputDebugStringA(std::format("vertexBufferView_.SizeInBytes={} useVertex={} 1vertex={} bytes\n", vertexBufferViewMetaball_.SizeInBytes, vertices.size(), sizeof(VertexData)).c_str());*/
	assert(vertices.size() * sizeof(VertexData) <= vertexBufferViewMetaball_.SizeInBytes);

	// バッファオーバーチェック
	if (vertices.size() > kMaxVertexCount) {
		OutputDebugStringA("DrawMesh: 頂点バッファサイズ超過！\n");
		assert(false && "DrawMesh: 頂点バッファサイズ超過！");
		return;
	}
	if (indices.size() > kMaxIndexCount) {
		OutputDebugStringA("DrawMesh: インデックスバッファサイズ超過！\n");
		assert(false && "DrawMesh: インデックスバッファサイズ超過！");
		return;
	}


	// 転送
	{
		VertexData* vtxData = nullptr;
		vertexResourceMetaball_->Map(0, nullptr, reinterpret_cast<void**>(&vtxData));
		memcpy(vtxData, vertices.data(), sizeof(VertexData) * vertices.size());
		vertexResourceMetaball_->Unmap(0, nullptr);
	}
	{
		uint32_t* idxData = nullptr;
		indexResourceMetaball_->Map(0, nullptr, reinterpret_cast<void**>(&idxData));
		memcpy(idxData, indices.data(), sizeof(uint32_t) * indices.size());
		indexResourceMetaball_->Unmap(0, nullptr);
	}


	// 描画時
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferViewMetaball_);
	commandList_->IASetIndexBuffer(&indexBufferViewMetaball_);

	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// シェーダやテクスチャは各自合わせてください
	// マテリアルや行列リソースなどセット
	// ここはDrawTriangleやDrawSphereの実装と同じでOK
	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);

	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]); // ここは用途に合わせて

	commandList_->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
}
void GameBase::DrawMesh(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world) {
	
	commandList_->SetGraphicsRootSignature(rootSignature.Get()); 
	// 0. PSO切り替え
	if (IsMetaBall_) {
		commandList_->SetPipelineState(graphicsPipelineStateWhite.Get()); // 白単色
	} else {
		commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get()); // 通常
	}
	
	// 頂点リソース作成
	vertexResourceMetaball_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * vertices.size());
	vertexBufferViewMetaball_.BufferLocation = vertexResourceMetaball_->GetGPUVirtualAddress();
	vertexBufferViewMetaball_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(vertices.size());
	vertexBufferViewMetaball_.StrideInBytes = sizeof(VertexData);

	// バッファオーバーチェック
	if (vertices.size() > kMaxVertexCount) {
		OutputDebugStringA("DrawMesh: 頂点バッファサイズ超過！\n");
		assert(false && "DrawMesh: 頂点バッファサイズ超過！");
		return;
	}

	// 頂点データ転送
	{
		VertexData* vtxData = nullptr;
		vertexResourceMetaball_->Map(0, nullptr, reinterpret_cast<void**>(&vtxData));
		memcpy(vtxData, vertices.data(), sizeof(VertexData) * vertices.size());
		vertexResourceMetaball_->Unmap(0, nullptr);
	}

	// WVP行列のセット（スロット2を使用）
	
	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / kClientHeight, 0.1f, 100.0f);
	Matrix4x4 wvpMatrix = function.Multiply(world, function.Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData[2].WVP = wvpMatrix;
	transformationMatrixData[2].World = world;

	// パイプライン設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferViewMetaball_);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ディスクリプタヒープとリソースの設定
	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);

	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress() + sizeof(TransformationMatrix) * 2); // スロット2を使用
	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]);                                                              // テクスチャ

	// 描画コマンド
	commandList_->DrawInstanced(static_cast<UINT>(vertices.size()), 1, 0, 0);
}

void GameBase::SetBlendMode(PSO::BlendMode blendMode){

	blendMode_ = blendMode;

}