#include "TxtManager.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>

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
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	return converter.from_bytes(text);
}