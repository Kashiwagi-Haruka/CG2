#include "JsonManager.h"
#include "Logger.h"
#include <fstream>

JsonManager* JsonManager::GetInstance() {
	static JsonManager instance;
	return &instance;
}

bool JsonManager::LoadJson(const std::string& filename) {
	data_.clear();

	std::ifstream file(filename);
	if (!file.is_open()) {
		Logger::Log("JSON ファイルを開けません\n");
		return false;
	}

	try {
		file >> data_;
	} catch (const nlohmann::json::parse_error& e) {
		Logger::Log(std::string("JSON の解析に失敗しました: ") + e.what() + "\n");
		data_.clear();
		return false;
	}

	return true;
}
bool JsonManager::SaveJson(const std::string& filename) const {
	std::ofstream file(filename);
	if (!file.is_open()) {
		Logger::Log("JSON ファイルを書き込めません\n");
		return false;
	}

	file << data_.dump(2);
	return true;
}
const nlohmann::json* JsonManager::Find(const std::string& key) const {
	if (!data_.is_object()) {
		return nullptr;
	}

	auto it = data_.find(key);
	if (it == data_.end()) {
		return nullptr;
	}

	return &(*it);
}