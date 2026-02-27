#include "Logger.h"

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>

namespace Logger {
namespace {
std::mutex gLogMutex;

std::ofstream& GetLogStream() {
	static std::ofstream stream;
	static bool initialized = false;
	if (!initialized) {
		std::error_code errorCode;
		std::filesystem::create_directories("Log", errorCode);
		stream.open("Log/Engine.log", std::ios::out | std::ios::app);
		initialized = true;
	}
	return stream;
}
} // namespace

void Log(const std::string& message) {
	std::lock_guard<std::mutex> lock(gLogMutex);
	OutputDebugStringA(message.c_str());

	std::ofstream& logStream = GetLogStream();
	if (logStream.is_open()) {
		logStream << message;
		logStream.flush();
	}
}

} // namespace Logger