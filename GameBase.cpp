#include "GameBase.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

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

	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事
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
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));

		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel: {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n"); // 初期化完了のログをだす
	DebugError();

	WindowClear();
	
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
	if (FAILED(hr)) {
		Log("Failed to create fence. HRESULT: " + std::to_string(hr));
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
	commandQueue = nullptr;
	commandQueueDesc = {};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 主に描画用途
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドアロケータを生成する
	commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	swapChain = nullptr;
	swapChainDesc = {};

	swapChainDesc.Width = kClientWidth;
	swapChainDesc.Height = kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	// ディスクリプタヒープの生成

	rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	
	// SRV用ディスクリプタヒープ作成
	srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);



	//rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
	//rtvDescriptorHeapDesc.NumDescriptors = 2;                    // ダブルバッファ用に2つ。多くてもかまわない
	//hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	//// ディスクリプタヒープがつくれなかったので起動できない
	//assert(SUCCEEDED(hr));
	DXCInitialize();

	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	// 上手く取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
	assert(backBufferIndex < 2);
	//
	assert(swapChainResources[0] != nullptr);
	assert(swapChainResources[1] != nullptr);
	assert(swapChainResources[backBufferIndex] != nullptr);
	//

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;      // 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	depthStenicilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);

	dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(depthStenicilResource, &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	// ImGui 初期化はここで！
	imguiM.MInitialize(hwnd, device, swapChainDesc, rtvDesc, srvDescriptorHeap);

	if (srvDescriptorHeap == nullptr) {
		assert(false);
	}
	texture_.Initialize(device, srvDescriptorHeap);
	GPUHandle_ = texture_.GetGpuHandle();
	assert(GPUHandle_.ptr != 0); // もし0なら SRV 作成に失敗してる

	DrawcommandList();


	CrtvTransitionBarrier();
	

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();

	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = {commandList};
	commandQueue->ExecuteCommandLists(1, commandLists);
	// GPUと05に画面の交換を行うよう通知する
	// GPUとOSに画面の交換を行うよう通知する
	swapChain->Present(1, 0);

	// Fenceを作る
	fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent != nullptr);

	// Fenceの値を更新
	fenceValue++;
	commandQueue->Signal(fence, fenceValue);

	// Fenceの値が指定Signalに達してるか確認
	if (fence->GetCompletedValue() < fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE); // ★ここでちゃんとGPUが終わるまで待つ！
	}

}
void GameBase::DebugLayer() {

#ifdef _DEBUG



	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
	//デバッグレイヤー
		debugController->EnableDebugLayer();
		//更にGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
	

#endif // DEBUG

}

void GameBase::DebugError() {

#ifdef _DEBUG

	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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

void GameBase::TransitionBarrier() {
	//transitionBarrierの設定
	
	


}

void GameBase::CrtvTransitionBarrier() {
	//画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	//今回はRenderTargetからPresentにする。
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
}

void GameBase::FenceEvent() {
	// Fenceの作成
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// ←ここ追加！！
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent != nullptr);
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
	
	texture_.Finalize();
	imguiM.Finalize();
	
	vertexResourceSphere->Release();

	if (vertexResourceSprite) {
		vertexResourceSprite->Release();
	}
	if (transformationMatrixResourceSprite) {
		transformationMatrixResourceSprite->Release();
	}


	if (transformResource) {
		//transformResource->Unmap(0, nullptr); // ちゃんと最後だけUnmapする
	}
	
	vertexResource->Release();
	materialResource->Release(); 
	transformResource->Release(); 

	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

	CloseHandle(fenceEvent);
	fence->Release();
	dsvDescriptorHeap->Release();
	depthStenicilResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();

//#ifdef _DEBUG
//	debugController->Release();
//#endif

	CloseWindow(hwnd);
}


void GameBase::DXCInitialize() {

	// dxcCompilerを初期化
	dxcUtils = nullptr;
	dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	PSO();
	VertexResource();
}

IDxcBlob* GameBase::CompileShader(/* CompilerするShaderファイルへのパス*/const std::wstring& filePath,
    // Compilerに使用するProfile
    const wchar_t* profile,
    // 初期化で生成したものをつかう
    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	// ここの中身をこの後書いていく
	// 1. hlslファイルを読む
	// // これからシェーダーをコンパイルする旨をログに出す
	Log(CStr->ConvertString_(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	// 読めなかったら止める
	assert(SUCCEEDED(hr));

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
	hr = dxcCompiler->Compile(
	    &shaderSourceBuffer,        // 読み込んだファイル
	    arguments,                  // コンパイルオプション
	    _countof(arguments),        // コンパイルオプションの数
	    includeHandler,             // includeが含まれた場合
	    IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));
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
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// 成功したログを出す
	Log(CStr->ConvertString_(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();

	// 実行用のバイナリを返却
	return shaderBlob;
}

void GameBase::PSO() {
	Log("PSO() Start\n");
	assert(device != nullptr);
	Log("device is OK\n");

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

	signatureBlob = nullptr;
	errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	

	assert(SUCCEEDED(hr));

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

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	 vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;                                                 // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                  // InputLayout
	graphicsPipelineStateDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};   // PixelShader
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
	graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}

void GameBase::VertexResource() {
	// 頂点リソース作成
	vertexResource = CreateBufferResource(device, sizeof(VertexData) * 6);
	assert(vertexResource != nullptr);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 左下
	vertexData[0].position = {-0.5f, -0.5f, 0.0f, 1.0f};
	vertexData[0].texcoord = {0.0f, 1.0f};

	// 上
	vertexData[1].position = {0.0f, 0.5f, 0.0f, 1.0f};
	vertexData[1].texcoord = {0.5f, 0.0f};

	// 右下
	vertexData[2].position = {0.5f, -0.5f, 0.0f, 1.0f};
	vertexData[2].texcoord = {1.0f, 1.0f};

	// 左下2
	vertexData[3].position = {-0.5f, -0.5f, 0.5f, 1.0f};
	vertexData[3].texcoord = {0.0f, 1.0f};

	// 上2
	vertexData[4].position = {0.0f, 0.0f, 0.0f, 1.0f};
	vertexData[4].texcoord = {0.5f, 0.0f};

	// 右下2
	vertexData[5].position = {0.5f, -0.5f, -0.5f, 1.0f};
	vertexData[5].texcoord = {1.0f, 1.0f};



	vertexResource->Unmap(0, nullptr);


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
	materialResource = CreateBufferResource(device, sizeof(Vector4));
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 赤

	// --- トランスフォーム用リソース ---
	transformResource = CreateBufferResource(device, sizeof(Matrix4x4));
	transformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData)); // ←ここ！！起動時にマップしっぱなし
	*transformationMatrixData = function.MakeIdentity(); // 初期値は単位行列
	Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
	*transformationMatrixData = worldViewProjectionMatrix;

	// --- Sprite用 頂点リソース ---
	vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);
	vertexBufferViewSprite={};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

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

	vertexResourceSprite->Unmap(0, nullptr);
	// Sprite用の TransformationMatrix リソース作成（1個分）
	transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(Matrix4x4));

	// データへのポインタ取得
	transformationMatrixDataSprite = nullptr;
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

	// 単位行列を書き込んでおく（初期状態）
	*transformationMatrixDataSprite = function.MakeIdentity();

	
		//// 通常の三角形の初期化（もともとの処理）
		//vertexResource = CreateBufferResource(device, sizeof(VertexData) * 3);
		//vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
		//vertexBufferView.SizeInBytes = sizeof(VertexData) * 3;
		//vertexBufferView.StrideInBytes = sizeof(VertexData);

		//VertexData* vertexData = nullptr;
		//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		//vertexData[0].position = {0.0f, 0.5f, 0.0f, 1.0f};
		//vertexData[0].texcoord = {0.5f, 0.0f};
		//vertexData[1].position = {0.5f, -0.5f, 0.0f, 1.0f};
		//vertexData[1].texcoord = {1.0f, 1.0f};
		//vertexData[2].position = {-0.5f, -0.5f, 0.0f, 1.0f};
		//vertexData[2].texcoord = {0.0f, 1.0f};
		//vertexResource->Unmap(0, nullptr);

		// 球体メッシュの追加（新規追加）
		const int kSubdivision = 16;
		const float pi = 3.14159265f;
		const float kLonEvery = pi * 2.0f / float(kSubdivision);
		const float kLatEvery = pi / float(kSubdivision);

		const int kVertexCount = kSubdivision * kSubdivision * 6;
	    kVertexCount_ = kVertexCount;
		vertexResourceSphere = CreateBufferResource(device, sizeof(VertexData) * kVertexCount);
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

				sphereVertexData[start + 0] = {
				    a, {u, v}
                };
				sphereVertexData[start + 1] = {
				    b, {u, v - (1.0f / kSubdivision)}
                };
				sphereVertexData[start + 2] = {
				    c, {u + (1.0f / kSubdivision), v}
                };
				sphereVertexData[start + 3] = {
				    c, {u + (1.0f / kSubdivision), v}
                };
				sphereVertexData[start + 4] = {
				    b, {u, v - (1.0f / kSubdivision)}
                };
				sphereVertexData[start + 5] = {
				    d, {u + (1.0f / kSubdivision), v - (1.0f / kSubdivision)}
                };
			}
		}
		vertexResourceSphere->Unmap(0, nullptr);

		//// その他の初期化（もともとの処理）
		//viewport = {0.0f, 0.0f, static_cast<float>(kClientWidth), static_cast<float>(kClientHeight), 0.0f, 1.0f};
		//scissorRect = {0, 0, kClientWidth, kClientHeight};

		//materialResource = CreateBufferResource(device, sizeof(Vector4));
		//Vector4* materialData = nullptr;
		//materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		//transformResource = CreateBufferResource(device, sizeof(Matrix4x4));
		//transformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
		//*transformationMatrixData = function.MakeIdentity();

		//Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		//Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		//Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
		//Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		//Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
		//*transformationMatrixData = worldViewProjectionMatrix;
	
}



ID3D12Resource* GameBase::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
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

	ID3D12Resource* bufferResource = nullptr;

	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	    D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadならこれ
	    nullptr, IID_PPV_ARGS(&bufferResource));

	if (FAILED(hr)) {
		return nullptr;
	}

	return bufferResource;
}

void GameBase::Update() {
	// --- 回転角度を更新（Y軸回転だけ）
	transform.rotate.y += 0.03f;

	// --- ワールド行列を作成（スケール → 回転 → 移動）
	Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
	*transformationMatrixData = worldViewProjectionMatrix;
	// Sprite用のワールド行列（スケール・回転・移動から生成）
	Matrix4x4 worldMatrixSprite = function.MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);

	// View行列（カメラ、今回は単位行列で固定）
	Matrix4x4 viewMatrixSprite = function.MakeIdentity();

	// 射影行列（平行投影、画面サイズで生成）
	Matrix4x4 projectionMatrixSprite = function.MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);

	// 最終WVP行列の計算と書き込み
	Matrix4x4 worldViewProjectionMatrixSprite = function.Multiply(worldMatrixSprite, function.Multiply(viewMatrixSprite, projectionMatrixSprite));

	*transformationMatrixDataSprite = worldViewProjectionMatrixSprite;
}

void GameBase::Draw() {
	// ★ここ！毎回リセットする
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	DrawcommandList();

	
	// ImGui フレーム開始
	imguiM.NewFrame();

	// --- ImGui ウィンドウ記述 ---
	ImGui::Begin("Debug Window");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();

	//ImGui::ShowDemoWindow();

	// ImGui 描画（SRVヒープとコマンドリストを渡す）
	imguiM.Render(srvDescriptorHeap, commandList);

	// RenderTarget → Present に戻す
	
	CrtvTransitionBarrier();

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();

	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = {commandList};
	commandQueue->ExecuteCommandLists(1, commandLists);
	// GPUと05に画面の交換を行うよう通知する
	// GPUとOSに画面の交換を行うよう通知する
	swapChain->Present(1, 0);

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();


	// Fenceで同期
	fenceValue++;
	commandQueue->Signal(fence, fenceValue);

	if (fence->GetCompletedValue() < fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

}



void GameBase::FrameStart() {
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));
}

ID3D12DescriptorHeap* GameBase::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}
ID3D12Resource* GameBase::CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {

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

	ID3D12Resource* resource = nullptr;
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	
	return resource;


}

void GameBase::DrawcommandList() {


	


	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックアップに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex];
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	assert(commandList != nullptr);

	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	// 描画先のRTVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f}; // 青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);       // Viewportを設定
	commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(graphicsPipelineState);                                         // PSOを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);                                     // VBVを設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());  // PixelShader側
	commandList->SetGraphicsRootConstantBufferView(1, transformResource->GetGPUVirtualAddress()); // VertexShader側
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap};
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	commandList->SetGraphicsRootDescriptorTable(2, GPUHandle_);

	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
	commandList->DrawInstanced(6, 1, 0, 0);
	
	
	// --- 球体描画 ---（追加すべき！）
	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);
	commandList->DrawInstanced(kVertexCount_, 1, 0, 0);


	    // VBV設定（スプライト用）
	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);

	// Transform（WVP）設定（ルートパラメータ1番目）
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

	// 描画（6頂点＝2枚三角形）
	commandList->DrawInstanced(6, 1, 0, 0);

}