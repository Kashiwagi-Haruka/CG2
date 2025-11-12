#include "ImGuiManager.h"
#include <dxgi1_6.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include <format>
#include <string>
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon,SrvManager* srvManager) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	srvIndex_ = srvManager->Allocate();
	ImGui_ImplDX12_Init(
	    dxCommon->GetDevice(), static_cast<int>(dxCommon->GetSwapChainResourcesNum()), dxCommon->GetRtvDesc().Format, srvManager->GetDescriptorHeap().Get(),
	    srvManager->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
	    srvManager->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	// ★ デバッグログ出力（ImGui が使う SRV のスロット確認）
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvManager->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvManager->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();

	std::string log = "[ImGui Init] SRV Heap Start -> "
	                  "CPU Handle: " +
	                  std::to_string(cpuHandle.ptr) + " | GPU Handle: " + std::to_string(gpuHandle.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}

void ImGuiManager::NewFrame() {

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();


}

void ImGuiManager::Render(SrvManager* srvManager, DirectXCommon* dxCommon) {
	if (!srvManager->GetDescriptorHeap().Get() || !dxCommon->GetCommandList()) {
		OutputDebugStringA("ImGui Render Error: srvDescriptorHeap_ or commandList_ is null\n");
		return;
	}
	ImGui::Render();
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvManager->GetDescriptorHeap().Get()};
	dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());
}

void ImGuiManager::Finalize() {

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


}