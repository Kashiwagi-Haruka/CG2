#pragma once
#include <Windows.h>
#include <cstdint>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "ConvertString.h"
//dxc
#include <dxcapi.h>


class GameBase {

private:
	WNDCLASS wc{};
	RECT wrc;
	
	MSG msg{};
	ConvertString* CStr{};
	std::wstring wstringValue = L"k";

	ID3D12Debug1* debugController = nullptr;
	IDXGIFactory7* dxgiFactory;
	HRESULT hr;
	HWND hwnd;
	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	IDXGIAdapter4* useAdapter = nullptr;
	ID3D12Device* device = nullptr;
	ID3D12CommandQueue* commandQueue;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	// SwapChainからResourceを引っ張ってくる。
	ID3D12Resource* swapChainResources[2] = {nullptr};
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex;
	// transitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};

	// 初期値0でFenceをつくる
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);


	int32_t kClientWidth = 1280;
	int32_t kClientHeight = 720;

	// DXC
	// dxcCompilerを初期化
	IDxcUtils* dxcUtils;
	IDxcCompiler3* dxcCompiler;

	IDxcIncludeHandler* includeHandler;
	IDxcBlobEncoding* ShaderSource;
	DxcBuffer shaderSourceBuffer;
	IDxcResult* shaderResult;
	IDxcBlob* shaderBlob;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootsSignature{};
	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob;
	ID3DBlob* errorBlob;

	// バイナリを基に作成
	ID3D12RootSignature* rootSignature;

public:

	

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void Log(const std::string& message);

	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	bool IsMsgQuit();

	void OutPutLog();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void WindowClear();

	void DebugLayer();
	void DebugError();
	void TransitionBarrier();

	void CrtvTransitionBarrier();

	void FenceEvent();

	void CheackResourceLeaks();

	void ResourceRelease();

	MSG* GetMsg() { return &msg; };

	void DXCInitialize();
	/// <summary>
	/// CompileShader関数
	/// </summary>
	/// <param name="filePath">CompilerするShaderファイルへのパス</param>
	/// <param name="profile">Compilerに使用するProfile</param>
	/// <param name="dxcUtils">初期化で使用するものを3つ</param>
	/// <param name="dxcCompiler"></param>
	/// <param name="includeHandler"></param>
	/// <returns></returns>
	IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

	void RootSignature();
	 


};

