#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "GameBase.h"


class DX12 {

	IDXGIFactory7* dxgiFactory;
	HRESULT hr;
	GameBase* gamebase;
	ConvertString* CStr{};
	ID3D12Device* device = nullptr;
	ID3D12CommandQueue* commandQueue;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1* swapChainDesc;

	int32_t kClientWidth;
	int32_t kClientHeight;
	HWND hwnd;

public:
	void Initialize();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void WindowClear();
};
