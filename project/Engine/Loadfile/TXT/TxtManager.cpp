#include "TxtManager.h"

#include <Windows.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
std::u32string Utf8ToU32String(const std::string& text) {
	if (text.empty()) {
		return {};
	}

	const int wideLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);

	if (wideLength <= 0) {
		throw std::runtime_error("TxtManager: Invalid UTF-8 text.");
	}

	std::wstring wideText(static_cast<size_t>(wideLength), L'\0');
	const int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), wideText.data(), wideLength);

	if (converted <= 0) {
		throw std::runtime_error("TxtManager: Failed to convert UTF-8 text.");
	}

	std::u32string result;
	result.reserve(wideText.size());

	for (size_t i = 0; i < wideText.size(); ++i) {
		const wchar_t wc = wideText[i];
		if (wc >= 0xD800 && wc <= 0xDBFF) {
			if (i + 1 >= wideText.size()) {
				throw std::runtime_error("TxtManager: Invalid UTF-16 surrogate pair.");
			}
			const wchar_t low = wideText[++i];
			if (low < 0xDC00 || low > 0xDFFF) {
				throw std::runtime_error("TxtManager: Invalid UTF-16 surrogate pair.");
			}
			const char32_t codePoint = ((static_cast<char32_t>(wc - 0xD800) << 10) | static_cast<char32_t>(low - 0xDC00)) + 0x10000;
			result.push_back(codePoint);
		} else if (wc >= 0xDC00 && wc <= 0xDFFF) {
			throw std::runtime_error("TxtManager: Invalid UTF-16 surrogate pair.");
		} else {
			result.push_back(static_cast<char32_t>(wc));
		}
	}

	return result;
}
} // namespace

TxtManager* TxtManager::GetInstance() {
	static TxtManager instance;
	return &instance;
}

std::string TxtManager::LoadTxtAsString(const std::string& filePath) const {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		throw std::runtime_error("TxtManager: Failed to open file: " + filePath);
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::u32string TxtManager::LoadTxtAsU32String(const std::string& filePath) const {
	const std::string text = LoadTxtAsString(filePath);
	return Utf8ToU32String(text);
}