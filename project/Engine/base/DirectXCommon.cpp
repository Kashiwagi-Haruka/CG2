#define NOMINMAX
#include "DirectXCommon.h"
#include <cassert>
#include "Logger.h"
#include "StringUtility.h"
#include <format>
#include <dxcapi.h>
#include <thread>
#include "SrvManager.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;


void DirectXCommon::initialize(WinApp* winApp) {
	assert(winApp);
	winApp_ = winApp;

	InitializeFixFPS();

	// デバイスの初期化
	DeviceInitialize();
	// コマンドリストの初期化
	CommandListInitialize();
	// スワップチェインの初期化
	SwapChainInitialize();
	// 深度バッファの生成
	DepthBufferCreate();
	// ディスクリプタヒープの生成
	DescriptorHeapCreate();
	// RTVの初期化
	RenderTargetViewInitialize();
	// DSVの初期化
	DepthStencilViewInitialize();
	// フェンスの生成
	FenceCreate();
	// ビューポートとシザー矩形の設定
	ViewportRectInitialize();
	ScissorRectInitialize();
	// DXCコンパイラの生成
	DXCCompilerCreate();
	// ImGuiの初期化
	/*ImGuiInitialize();*/

	// Build PSO and root signature now (required before recording draw calls)
	SetupPSO();

	VertexResource();

	DrawCommandList();

	CrtvTransitionBarrier();

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, commandLists);

	// GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);

	// Fenceの値が指定Signalに達してるか確認
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE); // ★ここでちゃんとGPUが終わるまで待つ！
	}
}

void DirectXCommon::InitializeFixFPS() {

	reference_ = std::chrono::steady_clock::now();


}
void DirectXCommon::UpdateFixFPS() {

	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));

	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	//　現在時刻を取得する
	
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// 　前回時刻からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒　(よりわずかに短い時間)経ってない場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒になるまでスリープする
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			//1マイクロスリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	reference_ = std::chrono::steady_clock::now();

}

void DirectXCommon::DebugLayer() {

#ifdef _DEBUG

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		// デバッグレイヤー
		debugController_->EnableDebugLayer();
		// 更にGPU側でもチェックを行うようにする
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}

#endif // DEBUG
}
void DirectXCommon::DeviceInitialize() {

	

	/// Debuglayer
	DebugLayer();

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
			Logger::Log(StringUtility::ConvertString_(std::format(L"Use Adapater: {}\n", adapterDesc.Description)));
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
		hr_ = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));

		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr_)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Logger::Log(std::format("FeatureLevel: {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Logger::Log("Complete create D3D12Device!!!\n"); // 初期化完了のログをだす
	DebugError();
}
void DirectXCommon::DebugError() {

#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバいエラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
		                              // https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-window-11
		                              D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
		D3D12_INFO_QUEUE_FILTER filter{};

		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージの表示を抑制させる
		infoQueue->PushStorageFilter(&filter);

		// 解放
		infoQueue.Reset();
	}

#endif // _DEBUG
}
void DirectXCommon::CommandListInitialize() {

	// コマンドキューを生成する
	commandQueue_ = nullptr;
	commandQueueDesc_ = {};
	commandQueueDesc_.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc_.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 主に描画用途
	hr_ = device_->CreateCommandQueue(&commandQueueDesc_, IID_PPV_ARGS(&commandQueue_));

	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

// 初期化時 (CreateDevice後)
	for (UINT i = 0; i < kFrameCount; i++) {
		hr_ = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators_[i]));
		assert(SUCCEEDED(hr_));
	}


	// commandAllocators_[0] を使って最初に作る
	hr_ = device_->CreateCommandList(
	    0, D3D12_COMMAND_LIST_TYPE_DIRECT,
	    commandAllocators_[0].Get(), // ★最初の1つを渡す
	    nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr_));

	
	
}
void DirectXCommon::SwapChainInitialize() {

	swapChain_ = nullptr;
	swapChainDesc_ = {};

	swapChainDesc_.Width = WinApp::kClientWidth;
	swapChainDesc_.Height = WinApp::kClientHeight;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する

	hr_ = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr_));

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}
void DirectXCommon::DepthBufferCreate() {
	// ================================
	// 1. 深度バッファリソースの設定
	// ================================
	D3D12_RESOURCE_DESC depthDesc{};
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Width = WinApp::kClientWidth;
	depthDesc.Height = WinApp::kClientHeight;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.MipLevels = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 深度24bit + ステンシル8bit
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// ================================
	// 2. 利用するヒープの設定
	// ================================
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	// ================================
	// 3. 深度値のクリア設定
	// ================================
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f; // デフォルトは最大深度
	clearValue.DepthStencil.Stencil = 0;  // ステンシルは0クリア

	// ================================
	// 4. 深度バッファリソースの生成
	// ================================
	hr_ = device_->CreateCommittedResource(
	    &heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 初期状態
	    &clearValue, IID_PPV_ARGS(&depthStenicilResource_));
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::DescriptorHeapCreate() {

	descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// ディスクリプタヒープの生成

	rtvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);



	dsvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


}
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr_ = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr_));

	return descriptorHeap;
}
void DirectXCommon::RenderTargetViewInitialize() {

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
	
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;      // 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc_, rtvHandles_[0]);
	// 2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc_, rtvHandles_[1]);
}
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSrvCpuDescriptorHandle(uint32_t index) {
	return GetCpuDescriptorHandle(srvDescriptorHeap_, descriptorSizeSRV_, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSrvGpuDescriptorHandle(uint32_t index) { 
	return GetGpuDescriptorHandle(srvDescriptorHeap_, descriptorSizeSRV_, index); 
}
void DirectXCommon::DepthStencilViewInitialize() {

	// depthStenicilResource_ は DepthBufferCreate() で作成済みのはず
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device_->CreateDepthStencilView(
		depthStenicilResource_.Get(),
		&dsvDesc,
		dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart()
	);
}
void DirectXCommon::DXCCompilerCreate() {

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



}
void DirectXCommon::FenceCreate() {
	// Fenceを作る
	fenceValue_ = 0;
	hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ != nullptr);

	// ここ重要：コマンドキューが必ず有効か確認
	assert(commandQueue_ != nullptr);

	fenceValue_++;
	hr_ = commandQueue_->Signal(fence_.Get(), fenceValue_);
	assert(SUCCEEDED(hr_));
}
void DirectXCommon::ViewportRectInitialize(){

	// ビューポートとシザー設定
	viewport_ = {};
	viewport_.Width = float(WinApp::kClientWidth);
	viewport_.Height = float(WinApp::kClientHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	

}
void DirectXCommon::ScissorRectInitialize(){

	scissorRect_ = {};
	scissorRect_.left = 0;
	scissorRect_.right = WinApp::kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = WinApp::kClientHeight;

}
void DirectXCommon::ImGuiInitialize(){


	// ImGui 初期化はここで！
	/*imguiM_.MInitialize(winApp_->GetHwnd(), device_.Get(), swapChainDesc_, rtvDesc_, srvDescriptorHeap_.Get());*/


}

void DirectXCommon::PreDraw() {

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
	/*imguiM_.NewFrame();*/
	
}
void DirectXCommon::PostDraw() {

	/*imguiM_.Render(srvDescriptorHeap_.Get(), commandList_.Get());*/

	// RenderTarget→Present に戻す
	CrtvTransitionBarrier(); // バリア遷移 :contentReference[oaicite:4]{index=4}:contentReference[oaicite:5]{index=5}

	// コマンドリストをクローズして実行
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));
	Microsoft::WRL::ComPtr<ID3D12CommandList> lists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, lists->GetAddressOf());

	UpdateFixFPS();

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
void DirectXCommon::FrameStart() {

// FrameStart
	frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
	hr_ = commandAllocators_[frameIndex_]->Reset();
	assert(SUCCEEDED(hr_));
	hr_ = commandList_->Reset(commandAllocators_[frameIndex_].Get(), nullptr);
	assert(SUCCEEDED(hr_));
}

void DirectXCommon::DrawCommandList() {

	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックアップに対して行う
	barrier_.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
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

	

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {srvDescriptorHeap_.Get()};
	//commandList_->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}

void DirectXCommon::CrtvTransitionBarrier() {
	// 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	// 今回はRenderTargetからPresentにする。
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);
}

// CPUハンドルを取得
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCpuDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	// ヒープの先頭ハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// インデックス分だけオフセット
	handle.ptr += static_cast<SIZE_T>(descriptorSize) * index;
	return handle;
}

// GPUハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGpuDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	// ヒープの先頭ハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	// インデックス分だけオフセット
	handle.ptr += static_cast<UINT64>(descriptorSize) * index;
	return handle;
}
Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(
    /* CompilerするShaderファイルへのパス*/ const std::wstring& filePath,
    // Compilerに使用するProfile
    const wchar_t* profile
    ) {
	// ここの中身をこの後書いていく
	// 1. hlslファイルを読む
	// // これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString_(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
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
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr_ = dxcCompiler_->Compile(
	    &shaderSourceBuffer,
	    arguments,
	    _countof(arguments),
	    includeHandler_.Get(),
	    IID_PPV_ARGS(&shaderResult)
	);

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr_));
	// 3. 警告・エラーがでていないか確認する
	// // 警告・エラーが出たらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
		assert(false);
	}
	// 4. Compile結果を受け取って返す
	// コンパイル結果から実行用のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr_ = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr_));

	// 成功したログを出す
	Logger::Log(StringUtility::ConvertString_(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// もう使わないリソースを解放
	shaderSource.Reset();
	shaderResult.Reset();

	// 実行用のバイナリを返却
	return shaderBlob;
}
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {
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
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(ID3D12Device* device, int width, int height) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT; // 深度フォーマット
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthResource;
	HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthResource));
	assert(SUCCEEDED(hr));

	return depthResource;
}
void DirectXCommon::SetDirectionalLightData(const DirectionalLight& directionalLight) {
	if (directionalLightResource_) {
		directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
		*directionalLightData_ = directionalLight;
		directionalLightResource_->Unmap(0, nullptr);
	}
}
void DirectXCommon::Finalize() {
	/*imguiM_.Finalize();*/

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
}
void DirectXCommon::SetupPSO() {
	Logger::Log("SetupPSO() Start\n");
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
	directionalLightResource_ = CreateBufferResource(sizeof(DirectionalLight));
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
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	hr_ = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	rootSignature_ = nullptr;
	hr_ = device_->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
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

	// --- DepthStencil ---
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  // 深度書き込みを有効
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 手前なら描画
	// depthStencilDesc.StencilEnable = false;                        // ステンシル不要なら false

	// --- 共通設定 ---
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = rootSignature_.Get();
	baseDesc.InputLayout = inputLayoutDesc;
	baseDesc.BlendState = blendModeManeger_.SetBlendMode(blendMode_);
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.DepthStencilState = depthStencilDesc;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// --- 通常PSO（裏面カリング） ---
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = CompileShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psBlob = CompileShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
	assert(vsBlob && psBlob);

	for (int i = 0; i < BlendMode::kCountOfBlendMode; i++) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = baseDesc;
		psoDesc.BlendState = blendModeManeger_.SetBlendMode(static_cast<BlendMode>(i));
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState = rasterizerDesc;
		hr_ = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr_));
	}

	//// --- メタボール用PSO（両面描画＋真っ白PS） ---
	//Microsoft::WRL::ComPtr<IDxcBlob> whitePSBlob = CompileShader(L"Resources/shader/WhitePS.hlsl", L"ps_6_0");
	//assert(whitePSBlob);

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC metaballPsoDesc = baseDesc;
	//metaballPsoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	//metaballPsoDesc.PS = {whitePSBlob->GetBufferPointer(), whitePSBlob->GetBufferSize()};
	//D3D12_RASTERIZER_DESC rasterizerDescMetaball{};
	//rasterizerDescMetaball.CullMode = D3D12_CULL_MODE_NONE; // 両面描画
	//rasterizerDescMetaball.FillMode = D3D12_FILL_MODE_SOLID;
	//metaballPsoDesc.RasterizerState = rasterizerDescMetaball;
	//hr_ = device_->CreateGraphicsPipelineState(&metaballPsoDesc, IID_PPV_ARGS(&graphicsPipelineStateWhite_));
	//assert(SUCCEEDED(hr_));

	SetupParticlePSO();

	Logger::Log("SetupPSO END \n");
}
void DirectXCommon::SetupParticlePSO() {
	// RootParameter 配列を用意 (Material, Texture, Instancing)
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// b0 : Material (PS)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// t0 : Texture (PS)
	D3D12_DESCRIPTOR_RANGE rangeTexture{};
	rangeTexture.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeTexture.NumDescriptors = 1;
	rangeTexture.BaseShaderRegister = 0; // t0
	rangeTexture.RegisterSpace = 0;      // space0
	rangeTexture.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &rangeTexture;

	// t0, space1 : InstancingData (VS)
	D3D12_DESCRIPTOR_RANGE rangeInstancing{};
	rangeInstancing.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeInstancing.NumDescriptors = 1;
	rangeInstancing.BaseShaderRegister = 1; // t0
	rangeInstancing.RegisterSpace = 0;      // space1
	rangeInstancing.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &rangeInstancing;

	// b1 : Transform (VS)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].Descriptor.ShaderRegister = 1; // b1

	// Sampler s0 : PS
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// RootSignature
	D3D12_ROOT_SIGNATURE_DESC descRootSig{};
	descRootSig.pParameters = rootParameters;
	descRootSig.NumParameters = _countof(rootParameters);
	descRootSig.pStaticSamplers = &sampler;
	descRootSig.NumStaticSamplers = 1;
	descRootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	hr_ = D3D12SerializeRootSignature(&descRootSig, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	hr_ = device_->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&particleRootSignature_));
	assert(SUCCEEDED(hr_));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElements[3] = {};
	inputElements[0] = {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[1] = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[2] = {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};

	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.pInputElementDescs = inputElements;
	inputLayout.NumElements = _countof(inputElements);

	// DepthStencil
	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// Shader
	auto vsBlob = CompileShader(L"Resources/shader/Particle.VS.hlsl", L"vs_6_0");
	auto psBlob = CompileShader(L"Resources/shader/Particle.PS.hlsl", L"ps_6_0");

	// PSO
	for (int i = 0; i < BlendMode::kCountOfBlendMode; i++) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = particleRootSignature_.Get();
		psoDesc.InputLayout = inputLayout;
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = blendModeManeger_.SetBlendMode(static_cast<BlendMode>(i));
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		psoDesc.DepthStencilState = depthDesc;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		hr_ = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&particlePipelineState_[i]));
		assert(SUCCEEDED(hr_));
	}

	Logger::Log("SetupParticlePSO END\n");
}

void DirectXCommon::CreateSphereResources() {
	const int kSubdivision = 16;
	const float pi = 3.14159265f;
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	const float kLatEvery = pi / float(kSubdivision);
	const int kVertexCount = kSubdivision * kSubdivision * 6;
	kVertexCount_ = kVertexCount;
	vertexBufferViewSphere_ = {};
	vertexResourceSphere_ = CreateBufferResource(sizeof(VertexData) * kVertexCount);
	vertexBufferViewSphere_.BufferLocation = vertexResourceSphere_->GetGPUVirtualAddress();
	vertexBufferViewSphere_.SizeInBytes = sizeof(VertexData) * kVertexCount;
	vertexBufferViewSphere_.StrideInBytes = sizeof(VertexData);

	VertexData* sphereVertexData = nullptr;
	vertexResourceSphere_->Map(0, nullptr, reinterpret_cast<void**>(&sphereVertexData));

	// ……球体生成ループ……
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
	vertexResourceSphere_->Unmap(0, nullptr);
}


void DirectXCommon::VertexResource() {

	// 頂点リソース作成
	

	// --- トランスフォーム用リソース ---
	// 配列サイズ: [0]=モデル/デバッグ, [1]=未使用, [2]～=球体用

	constexpr UINT kCBAlign = 256;
	UINT matrixAlignedSize = (sizeof(TransformationMatrix) + kCBAlign - 1) & ~(kCBAlign - 1);
	transformResource_ = CreateBufferResource(matrixAlignedSize * kMaxTransformSlots);

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData)); // 構造体配列として使う

	// 初期化は必ず構造体単位で
	for (int i = 0; i < kMaxTransformSlots; ++i) {
		transformationMatrixData[i].WVP = Function::MakeIdentity4x4();
		transformationMatrixData[i].World = Function::MakeIdentity4x4();
	}
	// インスタンシング用 TransformationMatrix リソース作成
	instancingResource_ = CreateBufferResource(sizeof(TransformationMatrix) * kNumInstance);

	// 書き込むためのポインタをマップ
	hr_ = instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	assert(SUCCEEDED(hr_) && instancingData_);

	// 単位行列を初期化
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		instancingData_[index].WVP = Function::MakeIdentity4x4();
		instancingData_[index].World = Function::MakeIdentity4x4();
	}


	CreateSphereResources();
	/*CreateModelResources();*/
}
void DirectXCommon::CreateInstancingSRV(SrvManager* srvManager) {
	assert(srvManager);

	uint32_t index = srvManager->Allocate();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = kNumInstance;
	desc.Buffer.StructureByteStride = sizeof(TransformationMatrix128);

	instancingSrvHandleCPU = srvManager->GetCPUDescriptorHandle(index);
	instancingSrvHandleGPU = srvManager->GetGPUDescriptorHandle(index);

	device_->CreateShaderResourceView(instancingResource_.Get(), &desc, instancingSrvHandleCPU);
}

void DirectXCommon::DrawSphere(const Vector3& center, float radius, uint32_t color, int textureHandle, const Matrix4x4& viewProj) {
	// 0. まずここでPSO切り替え

	commandList_->SetPipelineState(graphicsPipelineState_[blendMode_].Get()); // 通常
	
	// 1. マテリアル設定
	Material* mat = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat));
	mat->color = Vector4(((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f, ((color >> 24) & 0xFF) / 255.0f);
	mat->enableLighting = 1;
	mat->uvTransform = Function::MakeIdentity4x4();
	materialResource_->Unmap(0, nullptr);

	// 2. ワールド行列とWVP
	Matrix4x4 world = Function::MakeAffineMatrix(
	    {radius, radius, radius}, // スケール
	    {0, 0, 0},                // 回転
	    center                    // 平行移動
	);
	Matrix4x4 wvp = Function::Multiply(world, viewProj);
	Logger::Log(std::format("WVP = "));
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Logger::Log(std::format("{},", wvp.m[i][j]));
		}
		Logger::Log(std::format("\n"));
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
	OutputDebugStringA(std::format("Sphere #(slot){}: center=({:.2f},{:.2f},{:.2f}), r={:.2f}, viewProj[0][0]={:.2f}\n", slot, center.x, center.y, center.z, radius, viewProj.m[0][0]).c_str());

	// 5. 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferViewSphere_;
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
	Logger::Log(std::format("\n World matrix scale = ({}, {}, {})\n", tr.m[0][0], tr.m[1][1], tr.m[2][2])); // 実装に合わせて要調整
}
void DirectXCommon::DrawSphere(const Vector3& center, const Vector3& radius, const Vector3& rotation, uint32_t color, int textureHandle, const Matrix4x4& viewProj) {
	// 0. まずここでPSO切り替え
	commandList_->SetPipelineState(graphicsPipelineState_[blendMode_].Get()); // 通常
	

	// 1. マテリアル設定
	Material* mat = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat));
	mat->color = Vector4(((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, (color & 0xFF) / 255.0f, ((color >> 24) & 0xFF) / 255.0f);
	mat->enableLighting = 1;
	mat->uvTransform = Function::MakeIdentity4x4();
	materialResource_->Unmap(0, nullptr);

	// 2. ワールド行列とWVP
	Matrix4x4 world = Function::MakeAffineMatrix(
	    radius,   // スケール
	    rotation, // 回転
	    center    // 平行移動
	);
	Matrix4x4 wvp = Function::Multiply(world, viewProj);
	Logger::Log(std::format("WVP = "));
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Logger::Log(std::format("{},", wvp.m[i][j]));
		}
		Logger::Log(std::format("\n"));
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
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferViewSphere_;
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
	Logger::Log(std::format("\n World matrix scale = ({}, {}, {})\n", tr.m[0][0], tr.m[1][1], tr.m[2][2])); // 実装に合わせて要調整
}
void DirectXCommon::DrawMesh(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world){

	commandList_->SetGraphicsRootSignature(rootSignature_.Get());
	// 0. PSO切り替え

	commandList_->SetPipelineState(graphicsPipelineState_[blendMode_].Get()); // 通常

	// 頂点リソース作成
	vertexResourceMesh_ = CreateBufferResource(sizeof(VertexData) * vertices.size());
	vertexBufferViewMesh_.BufferLocation = vertexResourceMesh_->GetGPUVirtualAddress();
	vertexBufferViewMesh_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(vertices.size());
	vertexBufferViewMesh_.StrideInBytes = sizeof(VertexData);

	// バッファオーバーチェック
	if (vertices.size() > kMaxVertexCount) {
		OutputDebugStringA("DrawMesh: 頂点バッファサイズ超過！\n");
		assert(false && "DrawMesh: 頂点バッファサイズ超過！");
		return;
	}

	// 頂点データ転送
	{
		VertexData* vtxData = nullptr;
		vertexResourceMesh_->Map(0, nullptr, reinterpret_cast<void**>(&vtxData));
		memcpy(vtxData, vertices.data(), sizeof(VertexData) * vertices.size());
		vertexResourceMesh_->Unmap(0, nullptr);
	}

	// ---- ここで color を Material に反映 ----
	Vector4 colorVec = {
	    ((color >> 16) & 0xFF) / 255.0f, // R
	    ((color >> 8) & 0xFF) / 255.0f,  // G
	    (color & 0xFF) / 255.0f,         // B
	    ((color >> 24) & 0xFF) / 255.0f  // A
	};

	Material* matData = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&matData));
	matData->color = colorVec; // 引数 color を RGBA に変換して設定
	matData->enableLighting = 1;
	matData->uvTransform = Function::MakeIdentity4x4();
	materialResource_->Unmap(0, nullptr);

	// WVP行列のセット（スロット2を使用）

	Matrix4x4 cameraMatrix = Function::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Function::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = Function::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / WinApp::kClientHeight, 0.1f, 100.0f);
	Matrix4x4 wvpMatrix = Function::Multiply(world, Function::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData[2].WVP = wvpMatrix;
	transformationMatrixData[2].World = world;

	// パイプライン設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferViewMesh_);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ディスクリプタヒープとリソースの設定
	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);

	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress() + sizeof(TransformationMatrix) * 2); // スロット2を使用
	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]);                                                 // テクスチャ

	// 描画コマンド
	commandList_->DrawInstanced(static_cast<UINT>(vertices.size()), 1, 0, 0);
}

void DirectXCommon::DrawParticle(const std::vector<VertexData>& vertices, uint32_t color, uint32_t textureHandle, const Matrix4x4& wvp, const Matrix4x4& world, int instanceCount) {

	// パーティクル専用の RootSignature と PSO を使う（修正）
	commandList_->SetGraphicsRootSignature(particleRootSignature_.Get());
	commandList_->SetPipelineState(particlePipelineState_[blendMode_].Get()); // particle 用 PSO を使用

	// 頂点リソース作成
	vertexResourceMesh_ = CreateBufferResource(sizeof(VertexData) * vertices.size());
	vertexBufferViewMesh_.BufferLocation = vertexResourceMesh_->GetGPUVirtualAddress();
	vertexBufferViewMesh_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(vertices.size());
	vertexBufferViewMesh_.StrideInBytes = sizeof(VertexData);

	// バッファオーバーチェック
	if (vertices.size() > kMaxVertexCount) {
		OutputDebugStringA("DrawMesh: 頂点バッファサイズ超過！\n");
		assert(false && "DrawMesh: 頂点バッファサイズ超過！");
		return;
	}

	// 頂点データ転送
	{
		VertexData* vtxData = nullptr;
		vertexResourceMesh_->Map(0, nullptr, reinterpret_cast<void**>(&vtxData));
		memcpy(vtxData, vertices.data(), sizeof(VertexData) * vertices.size());
		vertexResourceMesh_->Unmap(0, nullptr);
	}

	// ---- ここで color を Material に反映 ----
	Vector4 colorVec = {
	    ((color >> 16) & 0xFF) / 255.0f, // R
	    ((color >> 8) & 0xFF) / 255.0f,  // G
	    (color & 0xFF) / 255.0f,         // B
	    ((color >> 24) & 0xFF) / 255.0f  // A
	};

	Material* matData = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&matData));
	matData->color = colorVec; // 引数 color を RGBA に変換して設定
	matData->enableLighting = 1;
	matData->uvTransform = Function::MakeIdentity4x4();
	materialResource_->Unmap(0, nullptr);

	// WVP行列のセット（スロット2を使用）

	Matrix4x4 cameraMatrix = Function::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Function::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = Function::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / WinApp::kClientHeight, 0.1f, 100.0f);
	Matrix4x4 wvpMatrix = Function::Multiply(world, Function::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData[2].WVP = wvpMatrix;
	transformationMatrixData[2].World = world;

	// パイプライン設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferViewMesh_);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ディスクリプタヒープとリソースの設定
	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);

// RootParameter[0] : Material
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// RootParameter[3] : Transform
	commandList_->SetGraphicsRootConstantBufferView(3, transformResource_->GetGPUVirtualAddress() + sizeof(TransformationMatrix128) * 2);

	// RootParameter[1] : Texture
	commandList_->SetGraphicsRootDescriptorTable(1, TextureGPUHandle_[textureHandle]);

	// RootParameter[2] : InstancingData
	commandList_->SetGraphicsRootDescriptorTable(2, instancingSrvHandleGPU);
	// インスタンシング用 SRV（particle 用 root sig では Vertex 用）

	// 描画コマンド
	commandList_->DrawInstanced(static_cast<UINT>(vertices.size()), instanceCount, 0, 0);
}
void DirectXCommon::SetBlendMode(BlendMode blendmode){
	blendMode_ = blendmode;
	// コマンドリストに適用するPSOをセット
	commandList_->SetPipelineState(graphicsPipelineState_[blendMode_].Get());
}