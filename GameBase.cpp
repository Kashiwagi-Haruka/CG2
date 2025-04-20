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
	
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};

	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
	rtvDescriptorHeapDesc.NumDescriptors = 2;                    // ダブルバッファ用に2つ。多くてもかまわない
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	// ディスクリプタヒープがつくれなかったので起動できない
	assert(SUCCEEDED(hr));
	

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
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension =D3D12_RTV_DIMENSION_TEXTURE2D;// 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr+device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	DXCInitialize();
	PSO();
	VertexResource();
	VertexBufferView();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	
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
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	// 指定した色で画面全体をクリアする
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f}; // 青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	
		ResourceCommand();

	
	
	CrtvTransitionBarrier();

// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
hr = commandList->Close();

assert(SUCCEEDED(hr));

// GPUにコマンドリストの実行を行わせる
ID3D12CommandList* commandLists[] = {commandList};
commandQueue->ExecuteCommandLists(1, commandLists);
// GPUと05に画面の交換を行うよう通知する
swapChain->Present(1, 0);
FenceEvent();



	

//Fenceの値を更新
fenceValue++;
//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
commandQueue->Signal(fence, fenceValue);
//Fenceの値が指定したSignal値にたどり着いているか確認する
//GetCompletedValueの初期値はFence作成時に渡した初期値
if (fence->GetCompletedValue() < fenceValue) {
	//指定したsignalにたどり着いてないので、たどり着くまで待つようにイベントを設定する
	fence->SetEventOnCompletion(fenceValue, fenceEvent);
	WaitForSingleObject(fenceEvent, INFINITE);
}



// 次のフレーム用のコマンドリストを準備
hr = commandAllocator->Reset();
assert(SUCCEEDED(hr));
hr = commandList->Reset(commandAllocator, nullptr);
assert(SUCCEEDED(hr));


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
	
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	
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
	TraiangleResourceRelease();
	CloseHandle(fenceEvent);
	fence->Release();
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
#ifdef _DEBUG
	debugController->Release();
#endif // _DEBUG

	CloseWindow(hwnd);
}

void GameBase::DXCInitialize() { 
	dxcUtils = nullptr;
	dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));




	

}

IDxcBlob* GameBase::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	// これからシェーダーをコンパイルする旨をログに出す
	Log(CStr->ConvertString_(std::format(L" Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	// hlslファイルを読み込む
	ShaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &ShaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	shaderSourceBuffer.Ptr = ShaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = ShaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR arguments[] = {
	    filePath.c_str(),
	    L"-E",
	    L"main",
	    L"-T",
	    profile,
	    L"-Zi",
	    L"-Qembed_debug",
	    L"-Od",
	    L"-Zpr",
	};
	shaderResult = nullptr;

	hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments), includeHandler, IID_PPV_ARGS(&shaderResult));

	assert(SUCCEEDED(hr));

	//警告・エラーがでたらログに出して止める

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
	
	Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
	assert(false);
	}
	//コンパイル結果から実行用のバイナル部分を取得
	shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(CStr->ConvertString_(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	//もう使わないリソース解放
	ShaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを変更
	return shaderBlob;
}

void GameBase::RootSignature() {
//RootSignature作成
	descriptionRootsSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[1] = {};	
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	descriptionRootsSignature.pParameters = rootParameters;
	descriptionRootsSignature.NumParameters = _countof(rootParameters);

	MaterialResource();

	//シリアライズしてバイナリにする
	signatureBlob=nullptr;
	errorBlob=nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootsSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを基に作成
	rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

}
void GameBase::InputLayout() { 
	
	
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

}
void GameBase::BlenderState() {

	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}
void GameBase::RasterizerState() {
	
	//裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
}
void GameBase::SCompile() { 
		// shaderをコンパイルする
	vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	    assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	    assert(pixelShaderBlob != nullptr);


}
void GameBase::PSO() {
	RootSignature();
	InputLayout();
	BlenderState();
	RasterizerState();
	SCompile();
	graphicsPipelineStateDesc.pRootSignature=rootSignature;                                                   // RootSignature
	graphicsPipelineStateDesc.InputLayout=inputLayoutDesc;                                                   // InputLayout
	graphicsPipelineStateDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};   // PixelShader
	graphicsPipelineStateDesc.BlendState=blendDesc;                                                           // BlendState
	graphicsPipelineStateDesc.RasterizerState =rasterizerDesc;                                                 // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ (形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask=D3D12_DEFAULT_SAMPLE_MASK;
	
	graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	if (FAILED(hr)) {
		Log(std::format("Failed to create GraphicsPipelineState. HRESULT: 0x{:08X}\n", static_cast<unsigned int>(hr)));
		assert(false);
	}
	assert(SUCCEEDED(hr));

}
void GameBase::VertexResource() {

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type=D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3; // リソースのサイズ。今回はVector4を3頂点分
	                                               // バッファの場合はこれらはさにする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(device,sizeof(Vector4)*3);
	hr=device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));



}

void GameBase::VertexBufferView() {
	
	//リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	//頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
	// 上
	 vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};
	// 右下
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};


	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = float(kClientWidth);
	viewport.Height = float(kClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;



}

void GameBase::ResourceCommand() {

	commandList->RSSetViewports(1, &viewport); // Viewportを設定
	commandList->RSSetScissorRects(1, &scissorRect);
	// Scirssorを設定
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(graphicsPipelineState);
	// PSOを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	// VBVを設定
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// 描画! (DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
	commandList->DrawInstanced(3, 1, 0, 0);

}

void GameBase::TraiangleResourceRelease() {

materialResource->Release();

	vertexResource->Release();
	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();




}

void GameBase::MaterialResource() {
	//マテリアル用のリソースをつくる。今回はcolor1つ分のサイズを用意する
	materialResource = CreateBufferResource(device, sizeof(Vector4));
	//マテリアルにデータを書き込む
	Vector4* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込んでみる
	*materialData = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

}
ID3D12Resource* GameBase::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	assert(device != nullptr); // デバイスが正しく初期化されていることを確認

	// ヒープのプロパティ設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPU から GPU へ転送する用途

	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes; // バッファサイズ
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN; // バッファはフォーマット不要
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// リソースの作成
	ID3D12Resource* buffer = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));

	if (FAILED(hr)) {
		assert(false && "Failed to create buffer resource!"); // エラー時のデバッグ
		return nullptr;
	}

	return buffer;
}