#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>

class WinApp;
class DirectXCommon;
class SrvManager;
class ImGuiManager {

	uint32_t srvIndex_;


public:

	void Initialize(WinApp* winapp, DirectXCommon* dxCommon, SrvManager* srvManager );
	void NewFrame();
	void Render(SrvManager* srvManager,DirectXCommon* dxCommon);

	void Finalize();
};