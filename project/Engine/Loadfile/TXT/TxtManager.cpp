#include "TxtManager.h"

#include <fstream>
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