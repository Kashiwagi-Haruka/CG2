#pragma once
#include <nlohmann/json.hpp>
#include <string>

class JsonManager {
public:
	static JsonManager* GetInstance();

	// JSON を読み込む
	bool LoadJson(const std::string& filename);

	// 読み込んだ JSON を返す
	const nlohmann::json& GetData() const { return data_; }
	
	// ルートオブジェクト内のキー検索
	const nlohmann::json* Find(const std::string& key) const;

private:
	JsonManager() = default;
	nlohmann::json data_;
};