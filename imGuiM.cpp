#include "imGuiM.h"
#include <dxgi1_6.h>

void imGuiM::MInitialize(HWND hwnd, ID3D12Device* device_, DXGI_SWAP_CHAIN_DESC1 swapChainDesc, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ID3D12DescriptorHeap* srvDescriptorHeap_) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device_, swapChainDesc.BufferCount, rtvDesc.Format, srvDescriptorHeap_, srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}

void imGuiM::NewFrame() {

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();


}

void imGuiM::Render(ID3D12DescriptorHeap* srvDescriptorHeap_, ID3D12GraphicsCommandList* commandList_) {
	if (!srvDescriptorHeap_ || !commandList_) {
		OutputDebugStringA("ImGui Render Error: srvDescriptorHeap_ or commandList_ is null\n");
		return;
	}
	ImGui::Render();
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap_};
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);
}

void imGuiM::Finalize() {

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


}