#pragma once

#include <string>

class TxtManager {
public:
	static TxtManager* GetInstance();

	// 指定したtxtファイルを読み込み、内容を1つの文字列として返す
	std::string LoadTxtAsString(const std::string& filePath) const;

private:
	TxtManager() = default;
};