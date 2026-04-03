#include "ScreenShot.h"

#include "DirectXCommon.h"

#include <DirectXTex.h>
#include <filesystem>

bool ScreenShot::SaveBackBuffer(DirectXCommon* dxCommon, const std::string& filePath) {
	if (dxCommon == nullptr || filePath.empty()) {
		return false;
	}

}