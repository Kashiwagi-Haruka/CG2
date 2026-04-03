#include "ScreenShot.h"

#include "DirectXCommon.h"

#include <DirectXTex.h>
#include <filesystem>
#include <vector>

bool ScreenShot::SaveBackBuffer(DirectXCommon* dxCommon, const std::string& filePath) {
	if (dxCommon == nullptr || filePath.empty()) {
		return false;
	}

	const int requiredSize = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0);
	if (requiredSize <= 0) {
		return false;
	}

	std::vector<wchar_t> wideBuffer(static_cast<size_t>(requiredSize));
	if (MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, wideBuffer.data(), requiredSize) <= 0) {
		return false;
	}

	std::filesystem::path outputPath(wideBuffer.data());
	if (outputPath.has_parent_path()) {
		std::error_code ec;
		std::filesystem::create_directories(outputPath.parent_path(), ec);
	}

	DirectX::ScratchImage capturedImage;
	const HRESULT captureResult = DirectX::CaptureTexture(dxCommon->GetCommandQueueRaw(), dxCommon->GetCurrentBackBufferResource(), false, capturedImage);
	if (FAILED(captureResult)) {
		return false;
	}

	const DirectX::Image* image = capturedImage.GetImage(0, 0, 0);
	if (image == nullptr) {
		return false;
	}

	const HRESULT saveResult = DirectX::SaveToWICFile(*image, DirectX::WIC_FLAGS_FORCE_SRGB, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), outputPath.c_str());
	return SUCCEEDED(saveResult);
}