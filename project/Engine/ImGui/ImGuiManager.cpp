#include "ImGuiManager.h"
#include <dxgi1_6.h>
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif
#include "DirectXCommon.h"
#include "SrvManager/SrvManager.h"
#include "WinApp.h"
#include <format>
#include <string>

void ImGuiManager::Initialize([[maybe_unused]] WinApp* winApp, [[maybe_unused]] DirectXCommon* dxCommon, [[maybe_unused]] SrvManager* srvManager) {
#ifdef USE_IMGUI
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
#endif
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::Begin("Performance")) {
		ImGui::Text("FPS: %.1f", io.Framerate);
	}
	ImGui::End();
#endif
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	ImGui::Render();
#endif
}

void ImGuiManager::Draw([[maybe_unused]] SrvManager* srvManager, [[maybe_unused]] DirectXCommon* dxCommon) {
#ifdef USE_IMGUI
	if (!srvManager->GetDescriptorHeap().Get() || !dxCommon->GetCommandList()) {
		OutputDebugStringA("ImGui Render Error: srvDescriptorHeap_ or commandList_ is null\n");
		return;
	}
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvManager->GetDescriptorHeap().Get()};
	dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

#endif
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

#endif
}