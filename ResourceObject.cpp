#include "ResourceObject.h"
#include <wrl.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>


void ResourceObject::LeakChecker(){

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
	}
#endif



}