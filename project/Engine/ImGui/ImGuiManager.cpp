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
	//assert(srvManager->GetDescriptorHeap().Get() && "ImGuiManager::Initialize() srvDescriptorHeap_ is null");
	//assert(dxCommon->GetDevice() && "ImGuiManager::Initialize() device_ is null");
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	//ImGui_ImplWin32_Init(winApp->GetHwnd());
	//ImGui_ImplDX12_Init(
	//    dxCommon->GetDevice(), static_cast<int>(dxCommon->GetSwapChainResourcesNum()), dxCommon->GetRtvDesc().Format, srvManager->GetDescriptorHeap().Get(),
	//    srvManager->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
	//    srvManager->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	//// ★ デバッグログ出力（ImGui が使う SRV のスロット確認）
	//D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvManager->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvManager->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();

	//std::string log = "[ImGui Init] SRV Heap Start -> "
	//                  "CPU Handle: " +
	//                  std::to_string(cpuHandle.ptr) + " | GPU Handle: " + std::to_string(gpuHandle.ptr) + "\n";
	//OutputDebugStringA(log.c_str());
	//  Setup Dear ImGui context
	
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// Win32 backend（先に）
		ImGui_ImplWin32_Init(winApp->GetHwnd());

		// ---- DX12 backend（新API）----
		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = dxCommon->GetDevice();
		init_info.CommandQueue = dxCommon->GetCommandQueue().Get();
		init_info.NumFramesInFlight = static_cast<int>(dxCommon->GetSwapChainResourcesNum());
		init_info.RTVFormat = dxCommon->GetRtvDesc().Format; // 固定値より安全
		init_info.SrvDescriptorHeap = srvManager->GetDescriptorHeap().Get();

		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
			auto* sm = reinterpret_cast<SrvManager*>(info->UserData);
			uint32_t index = sm->Allocate();
			*out_cpu_handle = sm->GetCPUDescriptorHandle(index);
			*out_gpu_handle = sm->GetGPUDescriptorHandle(index);
		};

		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {
			// 今回はフリーリスト無しなので何もしない
		};

		init_info.UserData = srvManager;

		// ★これを忘れずに！
		ImGui_ImplDX12_Init(&init_info);

		// 任意のフラグ（順序はどちらでもOK）
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	
}

void ImGuiManager::Begin() {
	
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();


}

void ImGuiManager::End() {
	
	ImGui::Render();
	
}

void ImGuiManager::Draw(SrvManager* srvManager,DirectXCommon* dxCommon) {
	if (!srvManager->GetDescriptorHeap().Get() || !dxCommon->GetCommandList()) {
		OutputDebugStringA("ImGui Render Error: srvDescriptorHeap_ or commandList_ is null\n");
		return;
	}
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