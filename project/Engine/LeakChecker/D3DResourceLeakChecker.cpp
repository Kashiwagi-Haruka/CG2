#include "D3DResourceLeakChecker.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl.h>

void D3DResourceLeakChecker::LeakChecker() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	}
#endif
}
