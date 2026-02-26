#include "D3DResourceLeakChecker.h"

#include "GameBase.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string MakeTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	std::tm localTime = {};
	localtime_s(&localTime, &time);

	std::ostringstream oss;
	oss << std::put_time(&localTime, "%Y%m%d_%H%M%S");
	return oss.str();
}

void WriteCallPath(std::ofstream& reportFile) {
	reportFile << "[Function Trace to application files]\n";
	reportFile << "1. Engine/LeakChecker/D3DResourceLeakChecker.cpp : D3DResourceLeakChecker::LeakChecker\n";
	reportFile << "2. application/Scene/Game/Game.cpp : Game::Finalize\n";
	reportFile << "3. application/main.cpp : WinMain (application entry point)\n\n";
	reportFile << "Related headers:\n";
	reportFile << "- Engine/LeakChecker/D3DResourceLeakChecker.h\n";
	reportFile << "- application/Scene/Game/Game.h\n\n";
}

std::string ToNarrow(const std::wstring& source) {
	if (source.empty()) {
		return {};
	}
	const int size = WideCharToMultiByte(CP_UTF8, 0, source.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (size <= 1) {
		return {};
	}
	std::string result(size - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, source.c_str(), -1, result.data(), size, nullptr, nullptr);
	return result;
}
} // namespace

void D3DResourceLeakChecker::LeakChecker() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug = nullptr;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> infoQueue = nullptr;

	const std::filesystem::path logDirectory = "Log";
	std::error_code errorCode;
	std::filesystem::create_directories(logDirectory, errorCode);

	const std::filesystem::path reportPath = logDirectory / ("D3DResourceLeakReport_" + MakeTimestamp() + ".txt");
	std::ofstream reportFile(reportPath, std::ios::out | std::ios::trunc);
	if (!reportFile.is_open()) {
		return;
	}

	reportFile << "D3D12 Leak Check Report\n";
	reportFile << "======================\n\n";
	WriteCallPath(reportFile);

	if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		reportFile << "Failed to acquire IDXGIDebug1. Leak information could not be collected.\n";
		return;
	}



	std::vector<std::string> liveObjectMessages;
	std::vector<std::string> errorMessages;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&infoQueue)))) {
		const UINT64 messageCount = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (UINT64 i = 0; i < messageCount; ++i) {
			SIZE_T messageLength = 0;
			if (FAILED(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength)) || messageLength == 0) {
				continue;
			}

			std::vector<char> bytes(messageLength);
			auto* message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.data());
			if (FAILED(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength))) {
				continue;
			}

			std::string description = message->pDescription ? message->pDescription : "";
			if (description.find("Live") != std::string::npos || description.find("Refcount") != std::string::npos || description.find("RefCount") != std::string::npos) {
				liveObjectMessages.push_back(description);
			}

			if (message->Severity == DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR || message->Severity == DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION) {
				errorMessages.push_back(description);
			}
		}
	}

	// 実際の Device インスタンスからも RefCount を直接取得
	ID3D12Device* currentDevice = GameBase::GetInstance()->GetD3D12Device();
	if (currentDevice) {
		ULONG deviceRefCount = currentDevice->AddRef();
		currentDevice->Release();

		UINT nameLength = 0;
		std::wstring deviceName;
		if (SUCCEEDED(currentDevice->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameLength, nullptr)) && nameLength > 0) {
			deviceName.resize(nameLength / sizeof(wchar_t));
			if (SUCCEEDED(currentDevice->GetPrivateData(WKPDID_D3DDebugObjectNameW, &nameLength, deviceName.data()))) {
				if (!deviceName.empty() && deviceName.back() == L'\0') {
					deviceName.pop_back();
				}
			}
		}

		std::ostringstream oss;
		oss << "Live ID3D12Device at " << currentDevice << ", Refcount: " << deviceRefCount;
		if (!deviceName.empty()) {
			oss << ", Name: " << ToNarrow(deviceName);
		}
		liveObjectMessages.push_back(oss.str());
	}

	reportFile << "[RefCount Summary]\n";
	reportFile << "Live object related message count: " << liveObjectMessages.size() << "\n\n";

	reportFile << "[Remaining RefCount / Live Objects]\n";
	if (liveObjectMessages.empty()) {
		reportFile << "No live object messages were found in IDXGIInfoQueue.\n";
	} else {
		for (size_t i = 0; i < liveObjectMessages.size(); ++i) {
			reportFile << i + 1 << ". " << liveObjectMessages[i] << "\n";
		}
	}

	reportFile << "\n[Error details]\n";
	if (errorMessages.empty()) {
		reportFile << "No DXGI error/corruption messages were reported.\n";
	} else {
		for (size_t i = 0; i < errorMessages.size(); ++i) {
			reportFile << i + 1 << ". " << errorMessages[i] << "\n";
		}
		reportFile << "\nError location (function call path): Game::Finalize -> D3DResourceLeakChecker::LeakChecker\n";
	}

	if (infoQueue) {
		infoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
	}
	// 既存のデバッグ出力にもリーク情報を表示
	debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
#else
	// Releaseビルドでは何もしない
#endif
}