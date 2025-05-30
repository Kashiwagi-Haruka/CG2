#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include <d3d12.h>
#include <dxgi1_6.h>

class imGuiM {

public:

	void MInitialize(HWND hwnd, ID3D12Device* device, DXGI_SWAP_CHAIN_DESC1 swapChainDesc, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ID3D12DescriptorHeap* srvDescriptorHeap_);
	void NewFrame();
	void Render(ID3D12DescriptorHeap* srvDescriptorHeap_, ID3D12GraphicsCommandList* commandList_);

	void Finalize();
};