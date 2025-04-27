#pragma once
#include <Windows.h>
#include <cstdint>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "ConvertString.h"
#include <dxcapi.h>
#include "Function.h"
#include "Vector4.h"

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


	D3D12_VIEWPORT viewport;
	// シザー矩形
	D3D12_RECT scissorRect;
	ID3D12RootSignature* rootSignature;
	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource;
	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob;
	ID3DBlob* errorBlob;
	IDxcBlob* pixelShaderBlob;
	IDxcBlob* vertexShaderBlob;
	IDxcUtils* dxcUtils;
	IDxcCompiler3* dxcCompiler;
	IDxcIncludeHandler* includeHandler;

		// バッファの設定
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_RESOURCE_DESC resourceDesc;
	ID3D12Resource* materialResource;

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
	IDxcBlob* CompileShader(// CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile,
	    // 初期化で生成したものをつかう
	    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);
	void PSO();

	void VertexResource();

	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
};

